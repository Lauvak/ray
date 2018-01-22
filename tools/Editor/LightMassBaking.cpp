// +----------------------------------------------------------------------
// | Project : ray.
// | All rights reserved.
// +----------------------------------------------------------------------
// | Copyright (c) 2013-2017.
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
#include "LightMassBaking.h"

#include <gl\glew.h>

_NAME_BEGIN

LightMassBaking::HemiContext::HemiContext() noexcept
{
	interpolationThreshold = 0;

	std::memset(&mesh, 0, sizeof(mesh));
	std::memset(&meshPosition, 0, sizeof(meshPosition));
	std::memset(&lightmap, 0, sizeof(lightmap));
	std::memset(&hemisphere, 0, sizeof(hemisphere));
}

LightMassBaking::HemiContext::~HemiContext() noexcept
{
	glDeleteTextures(1, &hemisphere.firstPass.weightsTexture);
	glDeleteBuffers(1, &hemisphere.transfer.pbo);

	glDeleteShader(hemisphere.downsamplePass.vs);
	glDeleteShader(hemisphere.downsamplePass.fs);
	glDeleteProgram(hemisphere.downsamplePass.programID);

	glDeleteShader(hemisphere.firstPass.vs);
	glDeleteShader(hemisphere.firstPass.fs);
	glDeleteProgram(hemisphere.firstPass.programID);

	glDeleteVertexArrays(1, &hemisphere.vao);
	glDeleteRenderbuffers(1, &hemisphere.fbDepth);
	glDeleteFramebuffers(2, hemisphere.fb);
	glDeleteTextures(2, hemisphere.fbTexture);
}

LightMassBaking::LightMassBaking() noexcept
	: _isStopped(false)
{
	_camera.view = float4x4::One;
	_camera.project = float4x4::One;
	_camera.viewProject = float4x4::One;
	_camera.world = float4x4::One;
}

LightMassBaking::~LightMassBaking() noexcept
{
	this->close();
}

bool
LightMassBaking::setup(const LightSampleParams& params) noexcept
{
	assert(params.hemisphereNear < params.hemisphereFar && params.hemisphereNear > 0.0f);
	assert(params.hemisphereSize == 16 || params.hemisphereSize == 32 || params.hemisphereSize == 64 || params.hemisphereSize == 128 || params.hemisphereSize == 256 || params.hemisphereSize == 512);
	assert(params.interpolationPasses >= 0 && params.interpolationPasses <= 8);
	assert(params.interpolationThreshold >= 0.0f);

	auto context = std::make_unique<HemiContext>();

	context->meshPosition.passCount = 1 + 3 * params.interpolationPasses;
	context->interpolationThreshold = params.interpolationThreshold;
	context->hemisphere.size = params.hemisphereSize;
	context->hemisphere.znear = params.hemisphereNear;
	context->hemisphere.zfar = params.hemisphereFar;
	context->hemisphere.clearColor.r = params.environmentColor.x;
	context->hemisphere.clearColor.g = params.environmentColor.y;
	context->hemisphere.clearColor.b = params.environmentColor.z;
	context->hemisphere.fbHemiCountX = 1536 / (3 * context->hemisphere.size);
	context->hemisphere.fbHemiCountY = 512 / context->hemisphere.size;

	std::uint32_t w[] = { context->hemisphere.fbHemiCountX * context->hemisphere.size * 3, context->hemisphere.fbHemiCountX * context->hemisphere.size / 2 };
	std::uint32_t h[] = { context->hemisphere.fbHemiCountY * context->hemisphere.size,     context->hemisphere.fbHemiCountY * context->hemisphere.size / 2 };

	glGenTextures(2, context->hemisphere.fbTexture);
	glGenFramebuffers(2, context->hemisphere.fb);
	glGenRenderbuffers(1, &context->hemisphere.fbDepth);

	glBindRenderbuffer(GL_RENDERBUFFER, context->hemisphere.fbDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, static_cast<GLsizei>(w[0]), static_cast<GLsizei>(h[0]));
	glBindFramebuffer(GL_FRAMEBUFFER, context->hemisphere.fb[0]);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, context->hemisphere.fbDepth);

	for (int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, context->hemisphere.fbTexture[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(w[i]), static_cast<GLsizei>(h[i]), 0, GL_RGBA, GL_FLOAT, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, context->hemisphere.fb[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, context->hemisphere.fbTexture[i], 0);
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			if (_lightMassListener)
				_lightMassListener->onMessage("Could not create framebuffer!");

			return false;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenVertexArrays(1, &context->hemisphere.vao);

	{
		const char *vs =
			"#version 150 core\n"
			"const vec2 ps[4] = vec2[](vec2(1, -1), vec2(1, 1), vec2(-1, -1), vec2(-1, 1));\n"
			"void main()\n"
			"{\n"
			"gl_Position = vec4(ps[gl_VertexID], 0, 1);\n"
			"}\n";
		const char *fs =
			"#version 150 core\n"
			"uniform sampler2D hemispheres;\n"
			"uniform sampler2D weights;\n"

			"layout(pixel_center_integer) in vec4 gl_FragCoord;\n"

			"out vec4 outColor;\n"

			"vec4 weightedSample(ivec2 h_uv, ivec2 w_uv, ivec2 quadrant)\n"
			"{\n"
			"vec4 sample = texelFetch(hemispheres, h_uv + quadrant, 0);\n"
			"vec2 weight = texelFetch(weights, w_uv + quadrant, 0).rg;\n"
			"return vec4(sample.rgb * weight.r, sample.a * weight.g);\n"
			"}\n"

			"vec4 threeWeightedSamples(ivec2 h_uv, ivec2 w_uv, ivec2 offset)\n"
			"{\n"
			"vec4 sum = weightedSample(h_uv, w_uv, offset);\n"
			"offset.x += 2;\n"
			"sum += weightedSample(h_uv, w_uv, offset);\n"
			"offset.x += 2;\n"
			"sum += weightedSample(h_uv, w_uv, offset);\n"
			"return sum;\n"
			"}\n"

			"void main()\n"
			"{\n"
			"vec2 in_uv = (gl_FragCoord.xy - vec2(0.5)) * vec2(6.0, 2.0) + vec2(0.01);\n"
			"ivec2 h_uv = ivec2(in_uv);\n"
			"ivec2 w_uv = ivec2(mod(in_uv, vec2(textureSize(weights, 0))));\n"
			"vec4 lb = threeWeightedSamples(h_uv, w_uv, ivec2(0, 0));\n"
			"vec4 rb = threeWeightedSamples(h_uv, w_uv, ivec2(1, 0));\n"
			"vec4 lt = threeWeightedSamples(h_uv, w_uv, ivec2(0, 1));\n"
			"vec4 rt = threeWeightedSamples(h_uv, w_uv, ivec2(1, 1));\n"
			"outColor = lb + rb + lt + rt;\n"
			"}\n";

		context->hemisphere.firstPass.vs = loadShader(GL_VERTEX_SHADER, vs);
		if (!context->hemisphere.firstPass.vs)
			return false;

		context->hemisphere.firstPass.fs = loadShader(GL_FRAGMENT_SHADER, fs);
		if (!context->hemisphere.firstPass.fs)
			return false;

		context->hemisphere.firstPass.programID = loadProgram(context->hemisphere.firstPass.vs, context->hemisphere.firstPass.fs, 0, 0);
		if (!context->hemisphere.firstPass.programID)
		{
			if (_lightMassListener)
				_lightMassListener->onMessage("Failed to loading the hemisphere first pass shader program.");

			return false;
		}

		context->hemisphere.firstPass.hemispheresTextureID = glGetUniformLocation(context->hemisphere.firstPass.programID, "hemispheres");
		context->hemisphere.firstPass.weightsTextureID = glGetUniformLocation(context->hemisphere.firstPass.programID, "weights");
	}

	{
		const char *vs =
			"#version 150 core\n"
			"const vec2 ps[4] = vec2[](vec2(1, -1), vec2(1, 1), vec2(-1, -1), vec2(-1, 1));\n"
			"void main()\n"
			"{\n"
			"gl_Position = vec4(ps[gl_VertexID], 0, 1);\n"
			"}\n";
		const char *fs =
			"#version 150 core\n"
			"uniform sampler2D hemispheres;\n"

			"out vec4 outColor;\n"

			"void main()\n"
			"{\n"
			"ivec2 h_uv = ivec2((gl_FragCoord.xy - vec2(0.5)) * 2.0 + vec2(0.01));\n"
			"vec4 lb = texelFetch(hemispheres, h_uv + ivec2(0, 0), 0);\n"
			"vec4 rb = texelFetch(hemispheres, h_uv + ivec2(1, 0), 0);\n"
			"vec4 lt = texelFetch(hemispheres, h_uv + ivec2(0, 1), 0);\n"
			"vec4 rt = texelFetch(hemispheres, h_uv + ivec2(1, 1), 0);\n"
			"outColor = lb + rb + lt + rt;\n"
			"}\n";

		context->hemisphere.downsamplePass.vs = loadShader(GL_VERTEX_SHADER, vs);
		if (!context->hemisphere.downsamplePass.vs)
			return false;

		context->hemisphere.downsamplePass.fs = loadShader(GL_FRAGMENT_SHADER, fs);
		if (!context->hemisphere.downsamplePass.fs)
			return false;

		context->hemisphere.downsamplePass.programID = loadProgram(context->hemisphere.downsamplePass.vs, context->hemisphere.downsamplePass.fs, 0, 0);
		if (!context->hemisphere.downsamplePass.programID)
		{
			if (_lightMassListener)
				_lightMassListener->onMessage("Error loading the hemisphere downsample pass shader program.");

			return false;
		}

		context->hemisphere.downsamplePass.hemispheresTextureID = glGetUniformLocation(context->hemisphere.downsamplePass.programID, "hemispheres");
	}

	glGenBuffers(1, &context->hemisphere.transfer.pbo);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, context->hemisphere.transfer.pbo);
	glBufferData(GL_PIXEL_PACK_BUFFER, context->hemisphere.fbHemiCountX * context->hemisphere.fbHemiCountY * 4 * sizeof(float), 0, GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	context->hemisphere.fbHemiToLightmapLocation = std::make_unique<int2[]>(context->hemisphere.fbHemiCountX * context->hemisphere.fbHemiCountY);
	context->hemisphere.transfer.fbHemiToLightmapLocation = std::make_unique<int2[]>(context->hemisphere.fbHemiCountX * context->hemisphere.fbHemiCountY);

	_ctx = std::move(context);

	if (params.hemisphereWeights)
		this->updateHemisphereWeights(params.hemisphereWeights);
	else
	{
		auto weights = std::make_unique<HemisphereWeight<float>[]>(params.hemisphereSize * params.hemisphereSize);
		math::makeHemisphereWeights(weights.get(), params.hemisphereSize);
		this->updateHemisphereWeights(weights.get());
	}

	return true;
}

void
LightMassBaking::close() noexcept
{
	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	_ctx.reset();
}

void
LightMassBaking::setLightMassListener(LightMassListenerPtr pointer) noexcept
{
	_lightMassListener = pointer;
}

LightMassListenerPtr
LightMassBaking::getLightMassListener() const noexcept
{
	return _lightMassListener;
}

void
LightMassBaking::setWorldTransform(const float4x4& transform) noexcept
{
	_camera.world = transform;
}

const float4x4&
LightMassBaking::getWorldTransform() const noexcept
{
	return _camera.world;
}

bool
LightMassBaking::isStopped() const noexcept
{
	return _isStopped;
}

bool
LightMassBaking::start() noexcept
{
	try
	{
		Viewportt<int> vp;

		std::size_t baseIndex = 0;

		while (!_isStopped && this->beginSampleHemisphere(vp.ptr()))
		{
			if (!this->doSampleHemisphere(vp, _camera.viewProject))
				return false;

			if (baseIndex != _ctx->meshPosition.triangle.baseIndex)
			{
				if (_lightMassListener)
					_lightMassListener->onBakingProgressing(this->getSampleProcess());

				baseIndex = _ctx->meshPosition.triangle.baseIndex;
			}

			if (!this->endSampleHemisphere())
				return false;
		}
	}
	catch (...)
	{
		this->close();
	}

	return true;
}

void
LightMassBaking::stop() noexcept
{
	if (!_isStopped)
		_isStopped = true;
}

void
LightMassBaking::setRenderTarget(float outLightmap[], int w, int h, int c)
{
	_ctx->lightmap.data = outLightmap;
	_ctx->lightmap.width = w;
	_ctx->lightmap.height = h;
	_ctx->lightmap.channels = c;
}

void
LightMassBaking::setGeometry(int positionsType, const void *positionsXYZ, int positionsStride, int lightmapCoordsType, const void *lightmapCoordsUV, int lightmapCoordsStride, int count, int indicesType, const void *indices)
{
	_ctx->mesh.positions = (const std::uint8_t*)positionsXYZ;
	_ctx->mesh.positionsType = positionsType;
	_ctx->mesh.positionsStride = positionsStride == 0 ? sizeof(float3) : positionsStride;
	_ctx->mesh.uvs = (const std::uint8_t*)lightmapCoordsUV;
	_ctx->mesh.uvsType = lightmapCoordsType;
	_ctx->mesh.uvsStride = lightmapCoordsStride == 0 ? sizeof(float2) : lightmapCoordsStride;
	_ctx->mesh.indicesType = indicesType;
	_ctx->mesh.indices = (const std::uint8_t*)indices;
	_ctx->mesh.count = count;

	_ctx->meshPosition.pass = 0;

	this->setSamplePosition(0);
}

void
LightMassBaking::setSamplePosition(std::size_t indicesTriangleBaseIndex)
{
	assert(_ctx->lightmap.width < std::numeric_limits<float>::max());
	assert(_ctx->lightmap.height < std::numeric_limits<float>::max());

	float2 uvMin(FLT_MAX, FLT_MAX);
	float2 uvMax(-FLT_MAX, -FLT_MAX);
	float2 uvScale((float)_ctx->lightmap.width, (float)_ctx->lightmap.height);

	_ctx->meshPosition.triangle.baseIndex = indicesTriangleBaseIndex;

	for (int i = 0; i < 3; i++)
	{
		std::size_t index;
		if (_ctx->mesh.indicesType == GL_UNSIGNED_INT)
			index = ((const std::uint32_t*)_ctx->mesh.indices + _ctx->meshPosition.triangle.baseIndex)[i];
		else if (_ctx->mesh.indicesType == GL_UNSIGNED_SHORT)
			index = ((const std::uint16_t*)_ctx->mesh.indices + _ctx->meshPosition.triangle.baseIndex)[i];
		else if (_ctx->mesh.indicesType == GL_UNSIGNED_BYTE)
			index = ((const std::uint8_t*)_ctx->mesh.indices + _ctx->meshPosition.triangle.baseIndex)[i];
		else
			index = _ctx->meshPosition.triangle.baseIndex + i;

		float3 p;
		if (_ctx->mesh.positionsType == GL_FLOAT)
			p = *(const float3*)(_ctx->mesh.positions + index * _ctx->mesh.positionsStride);
		else if (_ctx->mesh.positionsType == GL_UNSIGNED_INT)
			p = float3((const std::uint32_t*)(_ctx->mesh.positions + index * _ctx->mesh.positionsStride));
		else if (_ctx->mesh.positionsType == GL_UNSIGNED_SHORT)
			p = float3((const std::uint16_t*)(_ctx->mesh.positions + index * _ctx->mesh.positionsStride));
		else if (_ctx->mesh.positionsType == GL_UNSIGNED_BYTE)
			p = float3((const std::uint8_t*)(_ctx->mesh.positions + index * _ctx->mesh.positionsStride));
		else
			assert(false);

		float2 uv;
		if (_ctx->mesh.uvsType == GL_FLOAT)
			uv = *(const float2*)(_ctx->mesh.uvs + index * _ctx->mesh.uvsStride);
		else if (_ctx->mesh.uvsType == GL_UNSIGNED_SHORT)
			uv = float2((const std::uint16_t*)(_ctx->mesh.uvs + index * _ctx->mesh.uvsStride)) / (float)std::numeric_limits<std::uint16_t>::max();
		else if (_ctx->mesh.uvsType == GL_UNSIGNED_BYTE)
			uv = float2((const std::uint8_t*)(_ctx->mesh.uvs + index * _ctx->mesh.uvsStride)) / (float)std::numeric_limits<std::uint8_t>::max();
		else if (_ctx->mesh.uvsType == GL_UNSIGNED_INT)
		{
			float u = ((const std::uint32_t*)(_ctx->mesh.uvs + index * _ctx->mesh.uvsStride))[0] / (float)std::numeric_limits<std::uint32_t>::max();
			float v = ((const std::uint32_t*)(_ctx->mesh.uvs + index * _ctx->mesh.uvsStride))[1] / (float)std::numeric_limits<std::uint32_t>::max();
			uv = float2(u, v);
		}
		else
			assert(false);

		_ctx->meshPosition.triangle.uv[i] = uv * uvScale;
		_ctx->meshPosition.triangle.p[i] = _camera.world * p;

		uvMin = math::min(uvMin, _ctx->meshPosition.triangle.uv[i]);
		uvMax = math::max(uvMax, _ctx->meshPosition.triangle.uv[i]);
	}

	int2 bbMin((int2)math::floor(uvMin));
	int2 bbMax((int2)math::ceil(uvMax));

	_ctx->meshPosition.rasterizer.minx = math::max(bbMin.x - 1, 0);
	_ctx->meshPosition.rasterizer.miny = math::max(bbMin.y - 1, 0);
	_ctx->meshPosition.rasterizer.maxx = math::min(bbMax.x + 1, _ctx->lightmap.width);
	_ctx->meshPosition.rasterizer.maxy = math::min(bbMax.y + 1, _ctx->lightmap.height);
	_ctx->meshPosition.rasterizer.x = _ctx->meshPosition.rasterizer.minx + this->passOffsetX();
	_ctx->meshPosition.rasterizer.y = _ctx->meshPosition.rasterizer.miny + this->passOffsetY();

	assert(_ctx->meshPosition.rasterizer.minx < _ctx->meshPosition.rasterizer.maxx && _ctx->meshPosition.rasterizer.miny < _ctx->meshPosition.rasterizer.maxy);

	if (_ctx->meshPosition.rasterizer.x <= _ctx->meshPosition.rasterizer.maxx &&
		_ctx->meshPosition.rasterizer.y <= _ctx->meshPosition.rasterizer.maxy &&
		this->findFirstConservativeTriangleRasterizerPosition())
		_ctx->meshPosition.hemisphere.side = 0;
	else
		_ctx->meshPosition.hemisphere.side = 5;
}

int
LightMassBaking::leftOf(const float2& a, const float2& b, const float2& c)
{
	float2 lv = b - a;
	float2 rv = c - b;

	float x = lv.x * rv.y - lv.y * rv.x;
	return x < 0 ? -1 : x > 0;
}

int
LightMassBaking::convexClip(float2* poly, int nPoly, const float2* clip, int nClip, float2* res)
{
	int nRes = nPoly;
	int dir = leftOf(clip[0], clip[1], clip[2]);
	for (int i = 0, j = nClip - 1; i < nClip && nRes; j = i++)
	{
		if (i != 0)
			for (nPoly = 0; nPoly < nRes; nPoly++)
				poly[nPoly] = res[nPoly];
		nRes = 0;
		float2 v0 = poly[nPoly - 1];
		int side0 = leftOf(clip[j], clip[i], v0);
		if (side0 != -dir)
			res[nRes++] = v0;
		for (int k = 0; k < nPoly; k++)
		{
			float2 v1 = poly[k], x;
			int side1 = leftOf(clip[j], clip[i], v1);
			if (side0 + side1 == 0 && side0 && math::lineIntersection(clip[j], clip[i], v0, v1, x))
				res[nRes++] = x;
			if (k == nPoly - 1)
				break;
			if (side1 != -dir)
				res[nRes++] = v1;
			v0 = v1;
			side0 = side1;
		}
	}

	return nRes;
}

std::uint32_t
LightMassBaking::passStepSize()
{
	std::uint32_t shift = _ctx->meshPosition.passCount / 3 - (_ctx->meshPosition.pass - 1) / 3;
	std::uint32_t step = (1 << shift);
	assert(step > 0);
	return step;
}

std::uint32_t
LightMassBaking::passOffsetX()
{
	if (!_ctx->meshPosition.pass)
		return 0;
	int passType = (_ctx->meshPosition.pass - 1) % 3;
	std::uint32_t halfStep = passStepSize() >> 1;
	return passType != 1 ? halfStep : 0;
}

std::uint32_t
LightMassBaking::passOffsetY()
{
	if (!_ctx->meshPosition.pass)
		return 0;

	int passType = (_ctx->meshPosition.pass - 1) % 3;
	std::uint32_t halfStep = passStepSize() >> 1;
	return passType != 0 ? halfStep : 0;
}

bool
LightMassBaking::hasConservativeTriangleRasterizerFinished()
{
	return _ctx->meshPosition.rasterizer.y >= _ctx->meshPosition.rasterizer.maxy;
}

void
LightMassBaking::moveToNextPotentialConservativeTriangleRasterizerPosition()
{
	std::uint32_t step = passStepSize();

	_ctx->meshPosition.rasterizer.x += step;

	while (_ctx->meshPosition.rasterizer.x >= _ctx->meshPosition.rasterizer.maxx)
	{
		_ctx->meshPosition.rasterizer.x = _ctx->meshPosition.rasterizer.minx + this->passOffsetX();
		_ctx->meshPosition.rasterizer.y += step;

		if (this->hasConservativeTriangleRasterizerFinished())
			break;
	}
}

float*
LightMassBaking::getLightmapPixel(int x, int y)
{
	assert(x >= 0 && x < _ctx->lightmap.width && y >= 0 && y < _ctx->lightmap.height);
	return _ctx->lightmap.data + (y * _ctx->lightmap.width + x) * _ctx->lightmap.channels;
}

void
LightMassBaking::setLightmapPixel(int x, int y, float* in)
{
	assert(x >= 0 && x < _ctx->lightmap.width && y >= 0 && y < _ctx->lightmap.height);
	float *p = _ctx->lightmap.data + (y * _ctx->lightmap.width + x) * _ctx->lightmap.channels;
	for (int j = 0; j < _ctx->lightmap.channels; j++)
		*p++ = *in++;
}

bool
LightMassBaking::trySamplingConservativeTriangleRasterizerPosition()
{
	if (hasConservativeTriangleRasterizerFinished())
		return false;

	float *pixelValue = getLightmapPixel(_ctx->meshPosition.rasterizer.x, _ctx->meshPosition.rasterizer.y);
	for (int j = 0; j < _ctx->lightmap.channels; j++)
		if (pixelValue[j] != 0.0f)
			return false;

	if (_ctx->meshPosition.pass > 0)
	{
		float *neighbors[4];
		int neighborCount = 0;
		int neighborsExpected = 0;
		int d = (int)this->passStepSize() / 2;
		int dirs = ((_ctx->meshPosition.pass - 1) % 3) + 1;
		if (dirs & 1)
		{
			neighborsExpected += 2;
			if (_ctx->meshPosition.rasterizer.x - d >= _ctx->meshPosition.rasterizer.minx &&
				_ctx->meshPosition.rasterizer.x + d < _ctx->meshPosition.rasterizer.maxx)
			{
				neighbors[neighborCount++] = this->getLightmapPixel(_ctx->meshPosition.rasterizer.x - d, _ctx->meshPosition.rasterizer.y);
				neighbors[neighborCount++] = this->getLightmapPixel(_ctx->meshPosition.rasterizer.x + d, _ctx->meshPosition.rasterizer.y);
			}
		}
		if (dirs & 2)
		{
			neighborsExpected += 2;
			if (_ctx->meshPosition.rasterizer.y - d >= _ctx->meshPosition.rasterizer.miny &&
				_ctx->meshPosition.rasterizer.y + d < _ctx->meshPosition.rasterizer.maxy)
			{
				neighbors[neighborCount++] = this->getLightmapPixel(_ctx->meshPosition.rasterizer.x, _ctx->meshPosition.rasterizer.y - d);
				neighbors[neighborCount++] = this->getLightmapPixel(_ctx->meshPosition.rasterizer.x, _ctx->meshPosition.rasterizer.y + d);
			}
		}

		if (neighborCount == neighborsExpected)
		{
			float avg[4] = { 0 };
			for (int i = 0; i < neighborCount; i++)
				for (int j = 0; j < _ctx->lightmap.channels; j++)
					avg[j] += neighbors[i][j];
			float ni = 1.0f / neighborCount;
			for (int j = 0; j < _ctx->lightmap.channels; j++)
				avg[j] *= ni;

			bool interpolate = true;
			for (int i = 0; i < neighborCount; i++)
			{
				bool zero = true;
				for (int j = 0; j < _ctx->lightmap.channels; j++)
				{
					if (neighbors[i][j] != 0.0f)
						zero = false;
					if (fabs(neighbors[i][j] - avg[j]) > _ctx->interpolationThreshold)
						interpolate = false;
				}
				if (zero)
					interpolate = false;
				if (!interpolate)
					break;
			}

			if (interpolate)
			{
				setLightmapPixel(_ctx->meshPosition.rasterizer.x, _ctx->meshPosition.rasterizer.y, avg);
				return false;
			}
		}
	}

	float2 pixel[16];
	pixel[0].set(_ctx->meshPosition.rasterizer.x, _ctx->meshPosition.rasterizer.y);
	pixel[1].set(_ctx->meshPosition.rasterizer.x + 1, _ctx->meshPosition.rasterizer.y);
	pixel[2].set(_ctx->meshPosition.rasterizer.x + 1, _ctx->meshPosition.rasterizer.y + 1);
	pixel[3].set(_ctx->meshPosition.rasterizer.x, _ctx->meshPosition.rasterizer.y + 1);

	float2 res[16];
	int nRes = convexClip(pixel, 4, _ctx->meshPosition.triangle.uv, 3, res);
	if (nRes > 0)
	{
		float2 centroid = res[0];
		float area = res[nRes - 1].x * res[0].y - res[nRes - 1].y * res[0].x;
		for (int i = 1; i < nRes; i++)
		{
			centroid = centroid + res[i];
			area += res[i - 1].x * res[i].y - res[i - 1].y * res[i].x;
		}
		centroid = centroid / (float)nRes;
		area = std::abs(area / 2.0f);

		if (area > 0.0f)
		{
			float2 uv = math::barycentric(_ctx->meshPosition.triangle.uv[0], _ctx->meshPosition.triangle.uv[1], _ctx->meshPosition.triangle.uv[2], centroid);

			if (math::isfinite(uv))
			{
				float3 p0 = _ctx->meshPosition.triangle.p[0];
				float3 p1 = _ctx->meshPosition.triangle.p[1];
				float3 p2 = _ctx->meshPosition.triangle.p[2];
				float3 v1 = p1 - p0;
				float3 v2 = p2 - p0;
				_ctx->meshPosition.sample.position = p0 + v2 * uv.x + v1 * uv.y;
				_ctx->meshPosition.sample.direction = math::normalize(math::cross(v1, v2));

				if (math::isfinite(_ctx->meshPosition.sample.position) &&
					math::isfinite(_ctx->meshPosition.sample.direction) &&
					math::length2(_ctx->meshPosition.sample.direction) > 0.5f) // don't allow 0.0f. should always be ~1.0f
				{
					float3 up = float3::UnitY;
					if (std::abs(math::dot(up, _ctx->meshPosition.sample.direction)) > 0.8f)
						up = float3::UnitZ;

					float3 side = math::normalize(math::cross(up, _ctx->meshPosition.sample.direction));
					up = math::normalize(math::cross(side, _ctx->meshPosition.sample.direction));
					int rx = _ctx->meshPosition.rasterizer.x % 3;
					int ry = _ctx->meshPosition.rasterizer.y % 3;

					const float baseAngle = 0.03f * M_PI;
					const float baseAngles[3][3] = {
						{ baseAngle, baseAngle + 1.0f / 3.0f, baseAngle + 2.0f / 3.0f },
						{ baseAngle + 1.0f / 3.0f, baseAngle + 2.0f / 3.0f, baseAngle },
						{ baseAngle + 2.0f / 3.0f, baseAngle, baseAngle + 1.0f / 3.0f }
					};
					float phi = 2.0f * M_PI * baseAngles[ry][rx] + 0.1f * ((float)rand() / (float)RAND_MAX);
					_ctx->meshPosition.sample.up = math::normalize(side * cosf(phi) + up * sinf(phi));

					return true;
				}
			}
		}
	}

	return false;
}

bool
LightMassBaking::findFirstConservativeTriangleRasterizerPosition()
{
	while (!this->trySamplingConservativeTriangleRasterizerPosition())
	{
		this->moveToNextPotentialConservativeTriangleRasterizerPosition();
		if (this->hasConservativeTriangleRasterizerFinished())
			return false;
	}

	return true;
}

bool
LightMassBaking::findNextConservativeTriangleRasterizerPosition()
{
	this->moveToNextPotentialConservativeTriangleRasterizerPosition();
	return this->findFirstConservativeTriangleRasterizerPosition();
}

bool
LightMassBaking::updateHemisphereWeights(const HemisphereWeight<float>* weights)
{
	assert(_ctx && weights);

	if (_ctx->hemisphere.firstPass.weightsTexture == GL_NONE)
	{
		glGenTextures(1, &_ctx->hemisphere.firstPass.weightsTexture);
		if (!_ctx->hemisphere.firstPass.weightsTexture)
		{
			if (_lightMassListener)
				_lightMassListener->onMessage("Failed to create texture with hemisphere weights");

			return false;
		}

		glBindTexture(GL_TEXTURE_2D, _ctx->hemisphere.firstPass.weightsTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _ctx->hemisphere.size * 3, _ctx->hemisphere.size, 0, GL_RG, GL_FLOAT, weights);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, _ctx->hemisphere.firstPass.weightsTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _ctx->hemisphere.size * 3, _ctx->hemisphere.size, 0, GL_RG, GL_FLOAT, weights);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
	}

	return true;
}

void
LightMassBaking::beginProcessHemisphereBatch()
{
	if (!_ctx->hemisphere.fbHemiIndex)
		return;

	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(_ctx->hemisphere.vao);

	int fbRead = 0;
	int fbWrite = 1;

	int outHemiSize = _ctx->hemisphere.size / 2;
	glBindFramebuffer(GL_FRAMEBUFFER, _ctx->hemisphere.fb[fbWrite]);

	glViewport(0, 0, outHemiSize * _ctx->hemisphere.fbHemiCountX, outHemiSize * _ctx->hemisphere.fbHemiCountY);

	glUseProgram(_ctx->hemisphere.firstPass.programID);
	glUniform1i(_ctx->hemisphere.firstPass.hemispheresTextureID, 0);
	glUniform1i(_ctx->hemisphere.firstPass.weightsTextureID, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _ctx->hemisphere.fbTexture[fbRead]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _ctx->hemisphere.firstPass.weightsTexture);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(_ctx->hemisphere.downsamplePass.programID);
	glUniform1i(_ctx->hemisphere.downsamplePass.hemispheresTextureID, 0);

	while (outHemiSize > 1)
	{
		std::swap(fbRead, fbWrite);
		outHemiSize /= 2;
		glBindFramebuffer(GL_FRAMEBUFFER, _ctx->hemisphere.fb[fbWrite]);
		glViewport(0, 0, outHemiSize * _ctx->hemisphere.fbHemiCountX, outHemiSize * _ctx->hemisphere.fbHemiCountY);
		glBindTexture(GL_TEXTURE_2D, _ctx->hemisphere.fbTexture[fbRead]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	if (fbWrite == 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		std::swap(fbRead, fbWrite);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _ctx->hemisphere.fb[fbRead]);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _ctx->hemisphere.fb[fbWrite]);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(
			0, 0, _ctx->hemisphere.fbHemiCountX, _ctx->hemisphere.fbHemiCountY,
			0, 0, _ctx->hemisphere.fbHemiCountX, _ctx->hemisphere.fbHemiCountY,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, _ctx->hemisphere.fb[fbWrite]);
	}

	glBindBuffer(GL_PIXEL_PACK_BUFFER, _ctx->hemisphere.transfer.pbo);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, _ctx->hemisphere.fb[1]);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
	glReadPixels(0, 0, _ctx->hemisphere.fbHemiCountX, _ctx->hemisphere.fbHemiCountY, GL_RGBA, GL_FLOAT, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	_ctx->hemisphere.transfer.pboTransferStarted = true;
	_ctx->hemisphere.transfer.fbHemiCount = _ctx->hemisphere.fbHemiIndex;
	_ctx->hemisphere.transfer.fbHemiToLightmapLocation.swap(_ctx->hemisphere.fbHemiToLightmapLocation);
	_ctx->hemisphere.fbHemiIndex = 0;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
}

bool
LightMassBaking::finishProcessHemisphereBatch()
{
	if (!_ctx->hemisphere.transfer.pboTransferStarted)
		return true;

	glBindBuffer(GL_PIXEL_PACK_BUFFER, _ctx->hemisphere.transfer.pbo);

	float *hemi = (float*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	if (!hemi)
	{
		if (_lightMassListener)
			_lightMassListener->onMessage("Could not map hemisphere buffer.");

		return false;
	}

	std::uint32_t hemiIndex = 0;
	for (std::uint32_t hy = 0; hy < _ctx->hemisphere.fbHemiCountY; hy++)
	{
		for (std::uint32_t hx = 0; hx < _ctx->hemisphere.fbHemiCountX; hx++)
		{
			float *c = hemi + (hy * _ctx->hemisphere.fbHemiCountX + hx) * 4;
			float validity = c[3];

			const int2& lmUV = _ctx->hemisphere.transfer.fbHemiToLightmapLocation[hy * _ctx->hemisphere.fbHemiCountX + hx];
			float *lm = _ctx->lightmap.data + (lmUV.y * _ctx->lightmap.width + lmUV.x) * _ctx->lightmap.channels;
			if (!lm[0] && validity > 0.9)
			{
				float scale = 1.0f / validity;
				switch (_ctx->lightmap.channels)
				{
				case 1:
					lm[0] = std::max((c[0] + c[1] + c[2]) * scale / 3.0f, FLT_MIN);
					break;
				case 2:
					lm[0] = std::max((c[0] + c[1] + c[2]) * scale / 3.0f, FLT_MIN);
					lm[1] = 1.0f;
					break;
				case 3:
					lm[0] = std::max(c[0] * scale, FLT_MIN);
					lm[1] = std::max(c[1] * scale, FLT_MIN);
					lm[2] = std::max(c[2] * scale, FLT_MIN);
					break;
				case 4:
					lm[0] = std::max(c[0] * scale, FLT_MIN);
					lm[1] = std::max(c[1] * scale, FLT_MIN);
					lm[2] = std::max(c[2] * scale, FLT_MIN);
					lm[3] = 1.0f;
					break;
				default:
					assert(false);
					break;
				}
			}

			if (++hemiIndex == _ctx->hemisphere.transfer.fbHemiCount)
				goto done;
		}
	}
done:
	glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	_ctx->hemisphere.transfer.pboTransferStarted = false;

	return true;
}

void
LightMassBaking::updateSampleCamera(float3 pos, float3 dir, const float3& up, float l, float r, float b, float t, float n, float f)
{
	// view matrix: lookAt(pos, pos + dir, up)
	float3 side = math::cross(dir, up);

	//up = cross(side, dir);
	dir = -dir; pos = -pos;

	float4x4& view = _camera.view;
	view.a1 = side.x; view.a2 = up.x; view.a3 = dir.x; view.a4 = 0.0f;
	view.b1 = side.y; view.b2 = up.y; view.b3 = dir.y; view.b4 = 0.0f;
	view.c1 = side.z; view.c2 = up.z; view.c3 = dir.z; view.c4 = 0.0f;
	view.d1 = math::dot(side, pos); view.d2 = math::dot(up, pos); view.d3 = math::dot(dir, pos); view.d4 = 1.0f;

	// projection matrix: frustum(l, r, b, t, n, f)
	float4x4& proj = _camera.project;
	float ilr = 1.0f / (r - l), ibt = 1.0f / (t - b), ninf = -1.0f / (f - n), n2 = 2.0f * n;
	proj.a1 = n2 * ilr;      proj.a2 = 0.0f;          proj.a3 = 0.0f;           proj.a4 = 0.0f;
	proj.b1 = 0.0f;          proj.b2 = n2 * ibt;      proj.b3 = 0.0f;           proj.b4 = 0.0f;
	proj.c1 = (r + l) * ilr; proj.c2 = (t + b) * ibt; proj.c3 = (f + n) * ninf; proj.c4 = -1.0f;
	proj.d1 = 0.0f;         proj.d2 = 0.0f;         proj.d3 = f * n2 * ninf;  proj.d4 = 0.0f;

	_camera.viewProject = proj * view;
}

bool
LightMassBaking::updateSampleHemisphere(int* viewport)
{
	if (_ctx->meshPosition.hemisphere.side >= 5)
		return false;

	if (_ctx->meshPosition.hemisphere.side == 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, _ctx->hemisphere.fb[0]);

		if (_ctx->hemisphere.fbHemiIndex == 0)
		{
			glClearColor(_ctx->hemisphere.clearColor.r, _ctx->hemisphere.clearColor.g, _ctx->hemisphere.clearColor.b, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		_ctx->hemisphere.fbHemiToLightmapLocation[_ctx->hemisphere.fbHemiIndex].set(_ctx->meshPosition.rasterizer.x, _ctx->meshPosition.rasterizer.y);
	}

	std::uint32_t x = (_ctx->hemisphere.fbHemiIndex % _ctx->hemisphere.fbHemiCountX) * _ctx->hemisphere.size * 3;
	std::uint32_t y = (_ctx->hemisphere.fbHemiIndex / _ctx->hemisphere.fbHemiCountX) * _ctx->hemisphere.size;

	int size = _ctx->hemisphere.size;
	float znear = _ctx->hemisphere.znear;
	float zfar = _ctx->hemisphere.zfar;

	const float3& pos = _ctx->meshPosition.sample.position;
	const float3& dir = _ctx->meshPosition.sample.direction;
	const float3& up = _ctx->meshPosition.sample.up;
	float3 right = math::cross(dir, up);

	switch (_ctx->meshPosition.hemisphere.side)
	{
	case 0: // center
		viewport[0] = x;
		viewport[1] = y;
		viewport[2] = size;
		viewport[3] = size;
		this->updateSampleCamera(pos, dir, up, -znear, znear, -znear, znear, znear, zfar);
		break;
	case 1: // right
		viewport[0] = size + x;
		viewport[1] = y;
		viewport[2] = size / 2;
		viewport[3] = size;
		this->updateSampleCamera(pos, right, up, -znear, 0.0f, -znear, znear, znear, zfar);
		break;
	case 2: // left
		viewport[0] = size + x + size / 2;
		viewport[1] = y;
		viewport[2] = size / 2;
		viewport[3] = size;
		this->updateSampleCamera(pos, -right, up, 0.0f, znear, -znear, znear, znear, zfar);
		break;
	case 3: // down
		viewport[0] = 2 * size + x;
		viewport[1] = y + size / 2;
		viewport[2] = size;
		viewport[3] = size / 2;
		this->updateSampleCamera(pos, -up, dir, -znear, znear, 0.0f, znear, znear, zfar);
		break;
	case 4: // up
		viewport[0] = 2 * size + x;
		viewport[1] = y;
		viewport[2] = size;
		viewport[3] = size / 2;
		this->updateSampleCamera(pos, up, -dir, -znear, znear, -znear, 0.0f, znear, zfar);
		break;
	default:
		assert(false);
		break;
	}

	return true;
}

bool
LightMassBaking::beginSampleHemisphere(int* outViewport4)
{
	assert(_ctx->meshPosition.triangle.baseIndex < _ctx->mesh.count);

	while (!this->updateSampleHemisphere(outViewport4))
	{
		if (this->findNextConservativeTriangleRasterizerPosition())
		{
			_ctx->meshPosition.hemisphere.side = 0;
		}
		else
		{
			if (_ctx->meshPosition.triangle.baseIndex + 3 < _ctx->mesh.count)
			{
				this->setSamplePosition(_ctx->meshPosition.triangle.baseIndex + 3);
			}
			else
			{
				if (!this->finishProcessHemisphereBatch())
					return false;

				this->beginProcessHemisphereBatch();

				if (!this->finishProcessHemisphereBatch())
					return false;

				if (++_ctx->meshPosition.pass == _ctx->meshPosition.passCount)
				{
					_ctx->meshPosition.pass = 0;
					_ctx->meshPosition.triangle.baseIndex = _ctx->mesh.count;

					return false;
				}

				this->setSamplePosition(0);
			}
		}
	}

	return true;
}

bool
LightMassBaking::endSampleHemisphere()
{
	if (++_ctx->meshPosition.hemisphere.side == 5)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		if (++_ctx->hemisphere.fbHemiIndex == _ctx->hemisphere.fbHemiCountX * _ctx->hemisphere.fbHemiCountY)
		{
			if (!this->finishProcessHemisphereBatch())
				return false;

			this->beginProcessHemisphereBatch();
		}
	}

	return true;
}

float
LightMassBaking::getSampleProcess() noexcept
{
	float passProgress = (float)_ctx->meshPosition.triangle.baseIndex / (float)_ctx->mesh.count;
	return ((float)_ctx->meshPosition.pass + passProgress) / (float)_ctx->meshPosition.passCount;
}

std::uint32_t
LightMassBaking::loadShader(std::uint32_t type, const char* source)
{
	GLuint shader = glCreateShader(type);
	if (shader == GL_NONE)
	{
		if (_lightMassListener)
			_lightMassListener->onMessage("Could not create shader.");

		return GL_NONE;
	}

	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		if (_lightMassListener)
		{
			_lightMassListener->onMessage("Could not compile shader.");

			GLint length = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
			if (length)
			{
				std::string log(length, 0);
				glGetShaderInfoLog(shader, length, &length, (char*)log.data());

				if (_lightMassListener)
					_lightMassListener->onMessage(log);
			}
		}

		glDeleteShader(shader);
		return GL_NONE;
	}

	return shader;
}

std::uint32_t
LightMassBaking::loadProgram(std::uint32_t vs, std::uint32_t fs, const char** attributes, int attributeCount)
{
	assert(vs && fs);

	GLuint program = glCreateProgram();
	if (program == GL_NONE)
	{
		if (_lightMassListener)
			_lightMassListener->onMessage("Could not create program.");

		return GL_NONE;
	}

	glAttachShader(program, vs);
	glAttachShader(program, fs);

	for (int i = 0; i < attributeCount; i++)
		glBindAttribLocation(program, i, attributes[i]);

	glLinkProgram(program);

	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		if (_lightMassListener)
		{
			_lightMassListener->onMessage("Could not link program!");

			GLint length = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
			if (length)
			{
				std::string log((std::size_t)length, 0);
				glGetProgramInfoLog(program, length, &length, (GLchar*)log.data());

				_lightMassListener->onMessage(log);
			}
		}

		glDeleteProgram(program);

		return GL_NONE;
	}

	return program;
}

_NAME_END