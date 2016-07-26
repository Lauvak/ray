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
#if defined(_BUILD_RENDERER)
#include <ray/light_component.h>
#include <ray/game_server.h>
#include <ray/render_feature.h>

_NAME_BEGIN

__ImplementSubClass(LightComponent, RenderComponent, "Light")

LightComponent::LightComponent() noexcept
{
	_light = std::make_shared<Light>();
	_light->setOwnerListener(this);
}

LightComponent::~LightComponent() noexcept
{
	_light->setRenderScene(nullptr);
}

void
LightComponent::setLightRange(float range) noexcept
{
	_light->setLightRange(range);
}

void
LightComponent::setLightIntensity(float intensity) noexcept
{
	_light->setLightIntensity(intensity);
}

void
LightComponent::setLightAttenuation(const float3& atten) noexcept
{
	return _light->setLightAttenuation(atten);
}

void
LightComponent::setSpotInnerCone(float value) noexcept
{
	_light->setSpotInnerCone(value);
}

void
LightComponent::setSpotOuterCone(float value) noexcept
{
	_light->setSpotOuterCone(value);
}

float
LightComponent::getLightRange() const noexcept
{
	return _light->getLightRange();
}

float
LightComponent::getLightIntensity() const noexcept
{
	return _light->getLightIntensity();
}

const float3& 
LightComponent::getLightAttenuation() const noexcept
{
	return _light->getLightAttenuation();
}

const float2&
LightComponent::getSpotInnerCone() const noexcept
{
	return _light->getSpotInnerCone();
}

const float2&
LightComponent::getSpotOuterCone() const noexcept
{
	return _light->getSpotOuterCone();
}

void
LightComponent::setShadowMode(ShadowMode shadow) noexcept
{
	_light->setShadowMode(shadow);
}

ShadowMode
LightComponent::getShadowMode() const noexcept
{
	return _light->getShadowMode();
}

void
LightComponent::setShadowBias(float bias) noexcept
{
	_light->setShadowBias(bias);
}

float
LightComponent::getShadowBias() const noexcept
{
	return _light->getShadowBias();
}

void
LightComponent::setSubsurfaceScattering(bool enable) noexcept
{
	_light->setSubsurfaceScattering(enable);
}

bool
LightComponent::getSubsurfaceScattering() const noexcept
{
	return _light->getSubsurfaceScattering();
}

void 
LightComponent::setGlobalIllumination(bool enable) noexcept
{
	_light->setGlobalIllumination(enable);
}

bool 
LightComponent::getGlobalIllumination() const noexcept
{
	return _light->getGlobalIllumination();
}

void
LightComponent::setLightColor(const float3& color) noexcept
{
	_light->setLightColor(color);
}

const float3&
LightComponent::getLightColor() const noexcept
{
	return _light->getLightColor();
}

void
LightComponent::setLightType(LightType type) noexcept
{
	_light->setLightType(type);
}

LightType
LightComponent::getLightType() const noexcept
{
	return _light->getLightType();
}

void
LightComponent::load(iarchive& reader) noexcept
{
	std::string lightType;
	std::string shadowMode;
	float2 spot(5.0f, 40.0f);
	float3 lightColor(1, 1, 1);
	float3 lightAtten;
	float lightIntensity = 1.0f;
	float lightRange = 1.0f;
	float shadowBias = 0.0;
	bool subsurfaceScattering = false;
	bool enableGI = false;

	GameComponent::load(reader);

	if (reader.getValue("atten", lightAtten))
		this->setLightAttenuation(lightAtten);

	reader >> make_archive(lightIntensity, "intensity");
	reader >> make_archive(lightRange, "range");
	reader >> make_archive(lightColor, "color");
	reader >> make_archive(lightAtten, "atten");
	reader >> make_archive(lightType, "type");
	reader >> make_archive(subsurfaceScattering, "sss");
	reader >> make_archive(enableGI, "GI");
	reader >> make_archive(shadowBias, "bias");
	reader >> make_archive(shadowMode, "shadow");
	reader >> make_archive(spot, "spot");

	if (lightType == "sun")
		this->setLightType(LightType::LightTypeSun);
	else if (lightType == "directional")
		this->setLightType(LightType::LightTypeDirectional);
	else if (lightType == "point")
		this->setLightType(LightType::LightTypePoint);
	else if (lightType == "spot")
		this->setLightType(LightType::LightTypeSpot);
	else if (lightType == "ambient")
		this->setLightType(LightType::LightTypeAmbient);
	else
		this->setLightType(LightType::LightTypePoint);

	if (shadowMode == "hard")
		this->setShadowMode(ShadowMode::ShadowModeHard);
	else if (shadowMode == "soft")
		this->setShadowMode(ShadowMode::ShadowModeSoft);
	else
		this->setShadowMode(ShadowMode::ShadowModeNone);

	this->setLightColor(lightColor);
	this->setLightRange(lightRange);
	this->setLightIntensity(lightIntensity);
	this->setSpotInnerCone(spot.x);
	this->setSpotOuterCone(spot.y);
	this->setShadowBias(shadowBias);
	this->setSubsurfaceScattering(subsurfaceScattering);
	this->setGlobalIllumination(enableGI);
}

void
LightComponent::save(oarchive& write) noexcept
{
	RenderComponent::save(write);
}

GameComponentPtr
LightComponent::clone() const noexcept
{
	auto result = std::make_shared<LightComponent>();
	result->setName(this->getName());
	result->setActive(this->getActive());
	return result;
}

void
LightComponent::onActivate() noexcept
{
	this->addComponentDispatch(GameDispatchType::GameDispatchTypeMoveAfter, this);


	_light->setRenderScene(GameServer::instance()->getFeature<RenderFeature>()->getRenderScene());
	_light->setTransform(this->getGameObject()->getWorldTransform());
}

void
LightComponent::onDeactivate() noexcept
{
	this->removeComponentDispatch(GameDispatchType::GameDispatchTypeMoveAfter, this);

	_light->setRenderScene(nullptr);
}

void
LightComponent::onMoveAfter() noexcept
{
	_light->setTransform(this->getGameObject()->getWorldTransform());
}

_NAME_END

#endif