// +----------------------------------------------------------------------
// | Project : ray.
// | All rights reserved.
// +----------------------------------------------------------------------
// | Copyright (c) 2013-2016.
// +----------------------------------------------------------------------
// | * Redistribution and use of this software in source and binary forms,
// |   with or without modification, are permitted provided that the following
// |   conditions are met:
// |
// | * Redistributions of source code must retain the above
// |   copyright notice, this list of conditions and the
// |   following disclaimer.
// |
// | * Redistributions in binary form must reproduce the above
// |   copyright notice, this list of conditions and the
// |   following disclaimer in the documentation and/or other
// |   materials provided with the distribution.
// |
// | * Neither the name of the ray team, nor the names of its
// |   contributors may be used to endorse or promote products
// |   derived from this software without specific prior
// |   written permission of the ray team.
// |
// | THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// | "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// | LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// | A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// | OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// | SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// | LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// | DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// | THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// | (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// | OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// +----------------------------------------------------------------------
#include <ray/material_pass.h>
#include <ray/material.h>
#include <ray/graphics_device.h>
#include <ray/graphics_shader.h>
#include <ray/graphics_pipeline.h>
#include <ray/graphics_descriptor.h>

_NAME_BEGIN

__ImplementSubClass(MaterialPass, rtti::Interface, "MaterialPass")

MaterialParamBinding::MaterialParamBinding() noexcept
	: _needUpdate(true)
{
}

MaterialParamBinding::~MaterialParamBinding() noexcept
{
}

void 
MaterialParamBinding::setMaterialParam(MaterialParamPtr param) noexcept
{
	_param = param;
}

MaterialParamPtr 
MaterialParamBinding::getMaterialParam() const noexcept
{
	return _param;
}

void 
MaterialParamBinding::setGraphicsUniformSet(GraphicsUniformSetPtr uniformSet) noexcept
{
	_uniformSet = uniformSet;
}

GraphicsUniformSetPtr 
MaterialParamBinding::getGraphicsUniformSet() const noexcept
{
	return _uniformSet;
}

void 
MaterialParamBinding::needUpdate(bool needUpdate) noexcept
{
	_needUpdate = needUpdate;
}

bool
MaterialParamBinding::needUpdate() noexcept
{
	return _needUpdate;
}

void
MaterialParamBinding::onNeedUpdate() noexcept
{
	this->needUpdate(true);
}

MaterialPass::MaterialPass() noexcept
{
}

MaterialPass::~MaterialPass() noexcept
{
	this->close();
}

bool
MaterialPass::setup(Material& material) noexcept
{
	assert(_state);
	assert(_program);
	assert(_inputLayout);

	GraphicsDescriptorSetLayoutDesc descriptorSetLayoutDesc;
	descriptorSetLayoutDesc.setUniformComponents(_program->getActiveUniforms());
	descriptorSetLayoutDesc.setUniformBlockComponents(_program->getActiveUniformBlocks());
	_descriptorSetLayout = _program->getDevice()->createDescriptorSetLayout(descriptorSetLayoutDesc);
	if (!_descriptorSetLayout)
		return false;

	GraphicsPipelineDesc pipelineDesc;
	pipelineDesc.setGraphicsState(_state);
	pipelineDesc.setGraphicsProgram(_program);
	pipelineDesc.setGraphicsInputLayout(_inputLayout);
	pipelineDesc.setGraphicsDescriptorSetLayout(_descriptorSetLayout);
	_pipeline = _program->getDevice()->createRenderPipeline(pipelineDesc);
	if (!_pipeline)
		return false;

	GraphicsDescriptorPoolDesc  descriptorPoolDesc;
	descriptorPoolDesc.setMaxSets(1);
	for (auto& activeUniform : _program->getActiveUniforms())
	{
		auto type = activeUniform->getType();
		if (type == GraphicsUniformType::GraphicsUniformTypeStorageImage ||
			type == GraphicsUniformType::GraphicsUniformTypeSamplerImage ||
			type == GraphicsUniformType::GraphicsUniformTypeCombinedImageSampler)
		{
			descriptorPoolDesc.addGraphicsDescriptorPoolComponent(GraphicsDescriptorPoolComponent(activeUniform->getType(), 1));
		}
	}

	for (auto& activeUniformBlock : _program->getActiveUniformBlocks())
	{
		descriptorPoolDesc.addGraphicsDescriptorPoolComponent(GraphicsDescriptorPoolComponent(activeUniformBlock->getType(), 1));
	}		

	_descriptorPool = _program->getDevice()->createDescriptorPool(descriptorPoolDesc);

	GraphicsDescriptorSetDesc descriptorSetDesc;
	descriptorSetDesc.setGraphicsDescriptorSetLayout(_descriptorSetLayout);
	descriptorSetDesc.setGraphicsDescriptorPool(_descriptorPool);
	_descriptorSet = _program->getDevice()->createDescriptorSet(descriptorSetDesc);
	if (!_descriptorSet)
		return false;

	const auto& activeUniformSets = _descriptorSet->getGraphicsUniformSets();
	for (const auto& activeUniformSet : activeUniformSets)
	{
		auto activeUniform = activeUniformSet->getGraphicsUniform();
		auto uniformName = activeUniform->getName();
		if (activeUniform->getType() == GraphicsUniformType::GraphicsUniformTypeStorageImage ||
			activeUniform->getType() == GraphicsUniformType::GraphicsUniformTypeCombinedImageSampler ||
			activeUniform->getType() == GraphicsUniformType::GraphicsUniformTypeSamplerImage)
		{
			auto pos = uniformName.find_first_of("_X_");
			if (pos != std::string::npos)
				uniformName = uniformName.substr(0, pos);
		}
		else
		{
			auto pos = uniformName.find_first_of('[');
			if (pos != std::string::npos)
				uniformName = uniformName.substr(0, pos);
		}
		
		auto param = material.getParameter(uniformName);
		if (!param)
			continue;

		if (param->getType() == activeUniform->getType())
		{
			auto binding = std::make_unique<MaterialParamBinding>();
			binding->setGraphicsUniformSet(activeUniformSet);

			if (param->getSemantic())
			{
				param->getSemantic()->addParamListener(binding.get());
				binding->setMaterialParam(param->getSemantic());
			}
			else
			{
				param->addParamListener(binding.get());
				binding->setMaterialParam(param);
			}				

			_bindings.push_back(std::move(binding));
		}
	}

	return true;
}

void
MaterialPass::close() noexcept
{
	for (auto& it : _bindings)
	{
		it->getMaterialParam()->removeParamListener(it.get());
	}

	_bindings.clear();
	_pipeline.reset();
	_descriptorSet.reset();
	_descriptorSetLayout.reset();
}

void
MaterialPass::setName(const std::string& name) noexcept
{
	_name = name;
}

const std::string&
MaterialPass::getName() const noexcept
{
	return _name;
}

void
MaterialPass::setGraphicsState(GraphicsStatePtr state) noexcept
{
	_state = state;
}

void
MaterialPass::setGraphicsProgram(GraphicsProgramPtr program) noexcept
{
	_program = program;
}

void
MaterialPass::setGraphicsInputLayout(GraphicsInputLayoutPtr inputLayout) noexcept
{
	_inputLayout = inputLayout;
}

void
MaterialPass::setGraphicsDescriptorPool(GraphicsDescriptorPoolPtr descriptorPool) noexcept
{
	_descriptorPool = descriptorPool;
}

void
MaterialPass::setGraphicsDescriptorSetLayout(GraphicsDescriptorSetLayoutPtr descriptorSetLayout) noexcept
{
	_descriptorSetLayout = descriptorSetLayout;
}

GraphicsStatePtr
MaterialPass::getGraphicsState() const noexcept
{
	return _state;
}

GraphicsProgramPtr
MaterialPass::getGraphicsProgram() const noexcept
{
	return _program;
}

GraphicsInputLayoutPtr
MaterialPass::getGraphicsInputLayout() const noexcept
{
	return _inputLayout;
}

GraphicsDescriptorPoolPtr
MaterialPass::getGraphicsDescriptorPool() const noexcept
{
	return _descriptorPool;
}

GraphicsDescriptorSetLayoutPtr
MaterialPass::getGraphicsDescriptorSetLayout() const noexcept
{
	return _descriptorSetLayout;
}

GraphicsPipelinePtr
MaterialPass::getRenderPipeline() const noexcept
{
	return _pipeline;
}

GraphicsDescriptorSetPtr 
MaterialPass::getDescriptorSet() const noexcept
{
	return _descriptorSet;
}

void 
MaterialPass::update() noexcept
{
	for (auto& it : _bindings)
	{
		if (!it->needUpdate())
			continue;

		it->needUpdate(false);

		auto param = it->getMaterialParam();
		auto uniform = it->getGraphicsUniformSet();
		auto type = param->getType();
		switch (type)
		{
		case GraphicsUniformType::GraphicsUniformTypeBool:
			uniform->uniform1b(param->getBool());
			break;
		case GraphicsUniformType::GraphicsUniformTypeInt:
			uniform->uniform1i(param->getInt());
			break;
		case GraphicsUniformType::GraphicsUniformTypeInt2:
			uniform->uniform2i(param->getInt2());
			break;
		case GraphicsUniformType::GraphicsUniformTypeInt3:
			uniform->uniform3i(param->getInt3());
			break;
		case GraphicsUniformType::GraphicsUniformTypeInt4:
			uniform->uniform4i(param->getInt4());
			break;
		case GraphicsUniformType::GraphicsUniformTypeUInt:
			uniform->uniform1ui(param->getUInt());
			break;
		case GraphicsUniformType::GraphicsUniformTypeUInt2:
			uniform->uniform2ui(param->getUInt2());
			break;
		case GraphicsUniformType::GraphicsUniformTypeUInt3:
			uniform->uniform3ui(param->getUInt3());
			break;
		case GraphicsUniformType::GraphicsUniformTypeUInt4:
			uniform->uniform4ui(param->getUInt4());
			break;
		case GraphicsUniformType::GraphicsUniformTypeFloat:
			uniform->uniform1f(param->getFloat());
			break;
		case GraphicsUniformType::GraphicsUniformTypeFloat2:
			uniform->uniform2f(param->getFloat2());
			break;
		case GraphicsUniformType::GraphicsUniformTypeFloat3:
			uniform->uniform3f(param->getFloat3());
			break;
		case GraphicsUniformType::GraphicsUniformTypeFloat4:
			uniform->uniform4f(param->getFloat4());
			break;
		case GraphicsUniformType::GraphicsUniformTypeFloat2x2:
			uniform->uniform2fmat(param->getFloat2x2());
			break;
		case GraphicsUniformType::GraphicsUniformTypeFloat3x3:
			uniform->uniform3fmat(param->getFloat3x3());
			break;
		case GraphicsUniformType::GraphicsUniformTypeFloat4x4:
			uniform->uniform4fmat(param->getFloat4x4());
			break;
		case GraphicsUniformType::GraphicsUniformTypeIntArray:
			uniform->uniform1iv(param->getIntArray());
			break;
		case GraphicsUniformType::GraphicsUniformTypeInt2Array:
			uniform->uniform2iv(param->getInt2Array());
			break;
		case GraphicsUniformType::GraphicsUniformTypeInt3Array:
			uniform->uniform3iv(param->getInt3Array());
			break;
		case GraphicsUniformType::GraphicsUniformTypeInt4Array:
			uniform->uniform4iv(param->getInt4Array());
			break;
		case GraphicsUniformType::GraphicsUniformTypeUIntArray:
			uniform->uniform1uiv(param->getUIntArray());
			break;
		case GraphicsUniformType::GraphicsUniformTypeUInt2Array:
			uniform->uniform2uiv(param->getUInt2Array());
			break;
		case GraphicsUniformType::GraphicsUniformTypeUInt3Array:
			uniform->uniform3uiv(param->getUInt3Array());
			break;
		case GraphicsUniformType::GraphicsUniformTypeUInt4Array:
			uniform->uniform4uiv(param->getUInt4Array());
			break;
		case GraphicsUniformType::GraphicsUniformTypeFloatArray:
			uniform->uniform1fv(param->getFloatArray());
			break;
		case GraphicsUniformType::GraphicsUniformTypeFloat2Array:
			uniform->uniform2fv(param->getFloat2Array());
			break;
		case GraphicsUniformType::GraphicsUniformTypeFloat3Array:
			uniform->uniform3fv(param->getFloat3Array());
			break;
		case GraphicsUniformType::GraphicsUniformTypeFloat4Array:
			uniform->uniform4fv(param->getFloat4Array());
			break;
		case GraphicsUniformType::GraphicsUniformTypeFloat2x2Array:
			uniform->uniform2fmatv(param->getFloat2x2Array());
			break;
		case GraphicsUniformType::GraphicsUniformTypeFloat3x3Array:
			uniform->uniform3fmatv(param->getFloat3x3Array());
			break;
		case GraphicsUniformType::GraphicsUniformTypeFloat4x4Array:
			uniform->uniform4fmatv(param->getFloat4x4Array());
			break;
		case GraphicsUniformType::GraphicsUniformTypeSampler:
			break;
		case GraphicsUniformType::GraphicsUniformTypeSamplerImage:
			uniform->uniformTexture(param->getTexture(), param->getTextureSampler());
			break;
		case GraphicsUniformType::GraphicsUniformTypeCombinedImageSampler:
			uniform->uniformTexture(param->getTexture(), param->getTextureSampler());
			break;
		case GraphicsUniformType::GraphicsUniformTypeStorageImage:
			uniform->uniformTexture(param->getTexture(), param->getTextureSampler());
			break;
		case GraphicsUniformType::GraphicsUniformTypeStorageTexelBuffer:
			uniform->uniformBuffer(param->getBuffer());
			break;
		case GraphicsUniformType::GraphicsUniformTypeStorageBuffer:
			uniform->uniformBuffer(param->getBuffer());
			break;
		case GraphicsUniformType::GraphicsUniformTypeStorageBufferDynamic:
			uniform->uniformBuffer(param->getBuffer());
			break;
		case GraphicsUniformType::GraphicsUniformTypeUniformTexelBuffer:
			uniform->uniformBuffer(param->getBuffer());
			break;
		case GraphicsUniformType::GraphicsUniformTypeUniformBuffer:
			uniform->uniformBuffer(param->getBuffer());
			break;
		case GraphicsUniformType::GraphicsUniformTypeUniformBufferDynamic:
			uniform->uniformBuffer(param->getBuffer());
			break;
		default:
			break;
		}
	}
}

_NAME_END