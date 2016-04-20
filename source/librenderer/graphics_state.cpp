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
#include <ray/graphics_state.h>

_NAME_BEGIN

__ImplementSubInterface(GraphicsState, GraphicsChild, "GraphicsState")

GraphicsStateDesc::GraphicsStateDesc() noexcept
	: _blendEnable(false)
	, _blendSeparateEnable(false)
	, _scissorTestEnable(false)
	, _srgbEnable(false)
	, _multisampleEnable(false)
	, _rasterizerDiscardEnable(false)
	, _depthEnable(true)
	, _depthBoundsEnable(false)
	, _depthWriteEnable(true)
	, _depthBiasEnable(false)
	, _depthClampEnable(false)
	, _depthBiasClamp(false)
	, _stencilEnable(false)
	, _blendOp(GraphicsBlendOp::GraphicsBlendOpAdd)
	, _blendAlphaOp(GraphicsBlendOp::GraphicsBlendOpAdd)
	, _blendSrc(GraphicsBlendFactor::GraphicsBlendFactorSrcAlpha)
	, _blendDest(GraphicsBlendFactor::GraphicsBlendFactorOneMinusSrcAlpha)
	, _blendAlphaSrc(GraphicsBlendFactor::GraphicsBlendFactorSrcAlpha)
	, _blendAlphaDest(GraphicsBlendFactor::GraphicsBlendFactorOneMinusSrcAlpha)
	, _colorWriteMask(GraphicsColorMask::GraphicsColorMaskRGBA)
	, _cullMode(GraphicsCullMode::GraphicsCullModeBack)
	, _polygonMode(GraphicsPolygonMode::GraphicsPolygonModeSolid)
	, _primitiveType(GraphicsVertexType::GraphicsVertexTypeTriangleList)
	, _frontFace(GraphicsFrontFace::GraphicsFrontFaceCW)
	, _depthMin(0.0)
	, _depthMax(1.0)
	, _depthSlopeScaleBias(0)
	, _depthBias(0)
	, _depthFunc(GraphicsCompareFunc::GraphicsCompareFuncLequal)
	, _stencilFrontRef(0)
	, _stencilBackRef(0)
	, _stencilFrontReadMask(0xFFFFFFFF)
	, _stencilFrontWriteMask(0xFFFFFFFF)
	, _stencilBackReadMask(0xFFFFFFFF)
	, _stencilBackWriteMask(0xFFFFFFFF)
	, _stencilFrontFunc(GraphicsCompareFunc::GraphicsCompareFuncAlways)
	, _stencilBackFunc(GraphicsCompareFunc::GraphicsCompareFuncAlways)
	, _stencilFrontFail(GraphicsStencilOp::GraphicsStencilOpKeep)
	, _stencilFrontZFail(GraphicsStencilOp::GraphicsStencilOpKeep)
	, _stencilFrontPass(GraphicsStencilOp::GraphicsStencilOpKeep)	
	, _stencilBackFail(GraphicsStencilOp::GraphicsStencilOpKeep)
	, _stencilBackZFail(GraphicsStencilOp::GraphicsStencilOpKeep)
	, _stencilBackPass(GraphicsStencilOp::GraphicsStencilOpKeep)
{
}

GraphicsStateDesc::~GraphicsStateDesc() noexcept
{
}

void
GraphicsStateDesc::setBlendEnable(bool enable) noexcept
{
	_blendEnable = enable;
}

void
GraphicsStateDesc::setBlendSeparateEnable(bool enable) noexcept
{
	_blendSeparateEnable = enable;
}

void
GraphicsStateDesc::setBlendOp(GraphicsBlendOp blendOp) noexcept
{
	_blendOp = blendOp;
}

void
GraphicsStateDesc::setBlendSrc(GraphicsBlendFactor factor) noexcept
{
	_blendSrc = factor;
}

void
GraphicsStateDesc::setBlendDest(GraphicsBlendFactor factor) noexcept
{
	_blendDest = factor;
}

void
GraphicsStateDesc::setBlendAlphaOp(GraphicsBlendOp blendOp) noexcept
{
	_blendAlphaOp = blendOp;
}

void
GraphicsStateDesc::setBlendAlphaSrc(GraphicsBlendFactor factor) noexcept
{
	_blendAlphaSrc = factor;
}

void
GraphicsStateDesc::setBlendAlphaDest(GraphicsBlendFactor factor) noexcept
{
	_blendAlphaDest = factor;
}

void
GraphicsStateDesc::setColorWriteMask(GraphicsColorMask mask) noexcept
{
	_colorWriteMask = mask;
}

void
GraphicsStateDesc::setCullMode(GraphicsCullMode mode) noexcept
{
	_cullMode = mode;
}

void
GraphicsStateDesc::setPolygonMode(GraphicsPolygonMode mode) noexcept
{
	_polygonMode = mode;
}

void
GraphicsStateDesc::setPrimitiveType(GraphicsVertexType type) noexcept
{
	_primitiveType = type;
}

void
GraphicsStateDesc::setFrontFace(GraphicsFrontFace face) noexcept
{
	_frontFace = face;
}

void
GraphicsStateDesc::setScissorTestEnable(bool enable) noexcept
{
	_scissorTestEnable = enable;
}

void
GraphicsStateDesc::setLinear2sRGBEnable(bool enable) noexcept
{
	_srgbEnable = enable;
}

void
GraphicsStateDesc::setMultisampleEnable(bool enable) noexcept
{
	_multisampleEnable = enable;
}

void
GraphicsStateDesc::setRasterizerDiscardEnable(bool enable) noexcept
{
	_rasterizerDiscardEnable = enable;
}

void
GraphicsStateDesc::setDepthEnable(bool enable) noexcept
{
	_depthEnable = enable;
}

void
GraphicsStateDesc::setDepthWriteEnable(bool enable) noexcept
{
	_depthWriteEnable = enable;
}

void
GraphicsStateDesc::setDepthBoundsEnable(bool enable) noexcept
{
	_depthBoundsEnable = enable;
}

void
GraphicsStateDesc::setDepthMin(float min) noexcept
{
	_depthMin = min;
}

void
GraphicsStateDesc::setDepthMax(float max) noexcept
{
	_depthMax = max;
}

void
GraphicsStateDesc::setDepthFunc(GraphicsCompareFunc func) noexcept
{
	_depthFunc = func;
}

void
GraphicsStateDesc::setDepthBiasEnable(bool enable) noexcept
{
	_depthBiasEnable = enable;
}

void
GraphicsStateDesc::setDepthBias(float bias) noexcept
{
	_depthBias = bias;
}

void
GraphicsStateDesc::setDepthSlopeScaleBias(float bias) noexcept
{
	_depthSlopeScaleBias = bias;
}

void
GraphicsStateDesc::setDepthBiasClamp(bool bias) noexcept
{
	_depthBiasClamp = bias;
}

void
GraphicsStateDesc::setDepthClampEnable(bool enable) noexcept
{
	_depthClampEnable = enable;
}

void
GraphicsStateDesc::setStencilEnable(bool enable) noexcept
{
	_stencilEnable = enable;
}

void
GraphicsStateDesc::setStencilFrontRef(std::uint32_t ref) noexcept
{
	_stencilFrontRef = ref;
}

void
GraphicsStateDesc::setStencilFrontFunc(GraphicsCompareFunc func) noexcept
{
	_stencilFrontFunc = func;
}

void
GraphicsStateDesc::setStencilFrontReadMask(std::uint32_t mask) noexcept
{
	_stencilFrontReadMask = mask;
}

void
GraphicsStateDesc::setStencilFrontWriteMask(std::uint32_t mask) noexcept
{
	_stencilFrontWriteMask = mask;
}

void
GraphicsStateDesc::setStencilFrontFail(GraphicsStencilOp stencilOp) noexcept
{
	_stencilFrontFail = stencilOp;
}

void
GraphicsStateDesc::setStencilFrontZFail(GraphicsStencilOp stencilOp) noexcept
{
	_stencilFrontZFail = stencilOp;
}

void
GraphicsStateDesc::setStencilFrontPass(GraphicsStencilOp stencilOp) noexcept
{
	_stencilFrontPass = stencilOp;
}

void
GraphicsStateDesc::setStencilBackRef(std::uint32_t ref) noexcept
{
	_stencilBackRef = ref;
}

void
GraphicsStateDesc::setStencilBackFunc(GraphicsCompareFunc func) noexcept
{
	_stencilBackFunc = func;
}

void
GraphicsStateDesc::setStencilBackReadMask(std::uint32_t mask) noexcept
{
	_stencilBackReadMask = mask;
}

void
GraphicsStateDesc::setStencilBackWriteMask(std::uint32_t mask) noexcept
{
	_stencilBackWriteMask = mask;
}

void
GraphicsStateDesc::setStencilBackFail(GraphicsStencilOp stencilOp) noexcept
{
	_stencilBackFail = stencilOp;
}

void
GraphicsStateDesc::setStencilBackZFail(GraphicsStencilOp stencilOp) noexcept
{
	_stencilBackZFail = stencilOp;
}

void
GraphicsStateDesc::setStencilBackPass(GraphicsStencilOp stencilOp) noexcept
{
	_stencilBackPass = stencilOp;
}

bool
GraphicsStateDesc::getBlendEnable() const noexcept
{
	return _blendEnable;
}

bool
GraphicsStateDesc::getBlendSeparateEnable() const noexcept
{
	return _blendSeparateEnable;
}

GraphicsBlendOp
GraphicsStateDesc::getBlendOp() const noexcept
{
	return _blendOp;
}

GraphicsBlendFactor
GraphicsStateDesc::getBlendSrc() const noexcept
{
	return _blendSrc;
}

GraphicsBlendFactor
GraphicsStateDesc::getBlendDest() const noexcept
{
	return _blendDest;
}

GraphicsBlendOp
GraphicsStateDesc::getBlendAlphaOp() const noexcept
{
	return _blendAlphaOp;
}

GraphicsBlendFactor
GraphicsStateDesc::getBlendAlphaSrc() const noexcept
{
	return _blendAlphaSrc;
}

GraphicsBlendFactor
GraphicsStateDesc::getBlendAlphaDest() const noexcept
{
	return _blendAlphaDest;
}

GraphicsColorMask
GraphicsStateDesc::getColorWriteMask() const noexcept
{
	return _colorWriteMask;
}

GraphicsCullMode
GraphicsStateDesc::getCullMode() const noexcept
{
	return _cullMode;
}

GraphicsPolygonMode
GraphicsStateDesc::getPolygonMode() const noexcept
{
	return _polygonMode;
}

GraphicsVertexType
GraphicsStateDesc::getPrimitiveType() const noexcept
{
	return _primitiveType;
}

GraphicsFrontFace
GraphicsStateDesc::getFrontFace() const noexcept
{
	return _frontFace;
}

bool
GraphicsStateDesc::getScissorTestEnable() const noexcept
{
	return _scissorTestEnable;
}

bool
GraphicsStateDesc::getLinear2sRGBEnable() const noexcept
{
	return _srgbEnable;
}

bool
GraphicsStateDesc::getMultisampleEnable() const noexcept
{
	return _multisampleEnable;
}

bool
GraphicsStateDesc::getRasterizerDiscardEnable() const noexcept
{
	return _rasterizerDiscardEnable;
}

bool
GraphicsStateDesc::getDepthEnable() const noexcept
{
	return _depthEnable;
}

bool
GraphicsStateDesc::getDepthWriteEnable() const noexcept
{
	return _depthWriteEnable;
}

bool
GraphicsStateDesc::getDepthBoundsEnable() const noexcept
{
	return _depthBoundsEnable;
}

float
GraphicsStateDesc::getDepthMin() const noexcept
{
	return _depthMin;
}

float
GraphicsStateDesc::getDepthMax() const noexcept
{
	return _depthMax;
}

GraphicsCompareFunc
GraphicsStateDesc::getDepthFunc() const noexcept
{
	return _depthFunc;
}

bool
GraphicsStateDesc::getDepthBiasEnable() const noexcept
{
	return _depthBiasEnable;
}

float
GraphicsStateDesc::getDepthBias() const noexcept
{
	return _depthBias;
}

float
GraphicsStateDesc::getDepthSlopeScaleBias() const noexcept
{
	return _depthSlopeScaleBias;
}

bool
GraphicsStateDesc::getDepthBiasClamp() const noexcept
{
	return _depthBiasClamp;
}

bool
GraphicsStateDesc::getDepthClampEnable() const noexcept
{
	return _depthClampEnable;
}

bool
GraphicsStateDesc::getStencilEnable() const noexcept
{
	return _stencilEnable;
}

std::uint32_t
GraphicsStateDesc::getStencilFrontRef() const noexcept
{
	return _stencilFrontRef;
}

GraphicsCompareFunc
GraphicsStateDesc::getStencilFrontFunc() const noexcept
{
	return _stencilFrontFunc;
}

std::uint32_t
GraphicsStateDesc::getStencilFrontReadMask() const noexcept
{
	return _stencilFrontReadMask;
}

std::uint32_t
GraphicsStateDesc::getStencilFrontWriteMask() const noexcept
{
	return _stencilFrontWriteMask;
}

GraphicsStencilOp
GraphicsStateDesc::getStencilFrontFail() const noexcept
{
	return _stencilFrontFail;
}

GraphicsStencilOp
GraphicsStateDesc::getStencilFrontZFail() const noexcept
{
	return _stencilFrontZFail;
}

GraphicsStencilOp
GraphicsStateDesc::getStencilFrontPass() const noexcept
{
	return _stencilFrontPass;
}

std::uint32_t
GraphicsStateDesc::getStencilBackRef() const noexcept
{
	return _stencilBackRef;
}

GraphicsCompareFunc
GraphicsStateDesc::getStencilBackFunc() const noexcept
{
	return _stencilBackFunc;
}

std::uint32_t
GraphicsStateDesc::getStencilBackReadMask() const noexcept
{
	return _stencilBackReadMask;
}

std::uint32_t
GraphicsStateDesc::getStencilBackWriteMask() const noexcept
{
	return _stencilBackWriteMask;
}

GraphicsStencilOp
GraphicsStateDesc::getStencilBackFail() const noexcept
{
	return _stencilBackFail;
}

GraphicsStencilOp
GraphicsStateDesc::getStencilBackZFail() const noexcept
{
	return _stencilBackZFail;
}

GraphicsStencilOp
GraphicsStateDesc::getStencilBackPass() const noexcept
{
	return _stencilBackPass;
}

GraphicsState::GraphicsState() noexcept
{
}

GraphicsState::~GraphicsState() noexcept
{
}

_NAME_END