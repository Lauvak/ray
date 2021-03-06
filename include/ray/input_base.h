// +----------------------------------------------------------------------
// | Project : ray.
// | All rights reserved.
// +----------------------------------------------------------------------
// | Copyright (c) 2013-2015.
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
#ifndef _H_INPUT_BASE_H_
#define _H_INPUT_BASE_H_

#include <ray/input_device_base.h>
#include <ray/input_keyboard_base.h>
#include <ray/input_mouse_base.h>

_NAME_BEGIN

class EXPORT Input : public rtti::Interface
{
	__DeclareSubInterface(Input, rtti::Interface)
public:
	Input() noexcept;
	virtual ~Input() noexcept;

	virtual bool open() noexcept = 0;
	virtual bool open(InputDevicePtr& device) noexcept = 0;
	virtual bool open(InputDevicePtr&& device) noexcept = 0;
	virtual void close() noexcept = 0;

	virtual void setCaptureObject(CaptureObject window) noexcept = 0;
	virtual CaptureObject getCaptureObject() const noexcept = 0;

	virtual float getAxis(InputAxis::Code axis) const noexcept = 0;

	virtual void setMousePos(InputButton::mouse_t x, InputButton::mouse_t y) noexcept = 0;
	virtual void getMousePos(InputButton::mouse_t& x, InputButton::mouse_t& y) const noexcept = 0;

	virtual bool isKeyDown(InputKey::Code key) const noexcept = 0;
	virtual bool isKeyUp(InputKey::Code key) const noexcept = 0;
	virtual bool isKeyPressed(InputKey::Code key) const noexcept = 0;

	virtual bool isButtonDown(InputButton::Code key) const noexcept = 0;
	virtual bool isButtonUp(InputButton::Code key) const noexcept = 0;
	virtual bool isButtonPressed(InputButton::Code key) const noexcept = 0;

	virtual void showCursor(bool show) noexcept = 0;
	virtual bool isShowCursor() const noexcept = 0;

	virtual void lockCursor(bool lock) noexcept = 0;
	virtual bool isLockedCursor() const noexcept = 0;

	virtual void obtainMouseCapture() noexcept = 0;
	virtual void obtainKeyboardCapture() noexcept = 0;

	virtual void obtainMouseCapture(InputMousePtr& mouse) noexcept = 0;
	virtual void obtainMouseCapture(InputMousePtr&& mouse) noexcept = 0;
	virtual void obtainKeyboardCapture(InputKeyboardPtr& key) noexcept = 0;
	virtual void obtainKeyboardCapture(InputKeyboardPtr&& key) noexcept = 0;
	virtual void obtainCapture() noexcept = 0;

	virtual void releaseMouseCapture() noexcept = 0;
	virtual void releaseKeyboardCapture() noexcept = 0;
	virtual void releaseCapture() noexcept = 0;

	virtual void reset() noexcept = 0;

	virtual void addInputListener(InputListenerPtr& listener) noexcept = 0;
	virtual void addInputListener(InputListenerPtr&& listener) noexcept = 0;
	virtual void removeInputListener(InputListenerPtr& listener) noexcept = 0;
	virtual void removeInputListener(InputListenerPtr&& listener) noexcept = 0;
	virtual void clearInputListener() noexcept = 0;

	virtual bool sendInputEvent(const InputEvent& event) noexcept = 0;
	virtual bool postInputEvent(const InputEvent& event) noexcept = 0;

	virtual void updateBegin() noexcept = 0;
	virtual void update() noexcept = 0;
	virtual void updateEnd() noexcept = 0;

	virtual InputPtr clone() const noexcept = 0;

private:
	Input(const Input&) noexcept = delete;
	Input& operator=(const Input&) noexcept = delete;
};

_NAME_END

#endif