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
#ifndef _H_ARCHIVEBUF_H_
#define _H_ARCHIVEBUF_H_

#include <ray/iostream.h>
#include <ray/math.h>

_NAME_BEGIN

class archivebuf
{
public:
	archivebuf() noexcept;
	virtual ~archivebuf() noexcept;

	virtual std::string getCurrentNodeName() const noexcept = 0;
	virtual std::string getCurrentNodePath() const noexcept = 0;

	virtual bool addAttribute(const std::string& key, const std::string& value) noexcept = 0;
	virtual void setAttribute(const std::string& key, const std::string& value) noexcept = 0;
	virtual void removeAttribute(const std::string& key) noexcept = 0;

	virtual bool addNode(const std::string& key) noexcept = 0;
	virtual bool addSubNode(const std::string& key) noexcept = 0;

	virtual bool setToNode(const std::string& path) noexcept = 0;
	virtual bool setToFirstChild() noexcept = 0;
	virtual bool setToFirstChild(const std::string& name) noexcept = 0;
	virtual bool setToNextChild() noexcept = 0;
	virtual bool setToNextChild(const std::string& name) noexcept = 0;
	virtual bool setToParent() noexcept = 0;
	virtual bool setToRoot() noexcept = 0;

	virtual bool hasChild() const noexcept = 0;

	virtual bool hasAttr(const char* name) const noexcept = 0;
	virtual void clearAttrs() noexcept = 0;
	virtual bool addAttrs() noexcept = 0;
	virtual bool addAttrsInChildren() noexcept = 0;
	virtual bool addAttrsInChildren(const std::string& key) noexcept = 0;
	virtual const std::vector<std::string>& getAttrList() const noexcept = 0;

	virtual std::string getText() const noexcept = 0;

	virtual bool getValue(const std::string& name, bool& result) const noexcept = 0;
	virtual bool getValue(const std::string& name, int1& result) const noexcept = 0;
	virtual bool getValue(const std::string& name, int2& result) const noexcept = 0;
	virtual bool getValue(const std::string& name, int3& result) const noexcept = 0;
	virtual bool getValue(const std::string& name, int4& result) const noexcept = 0;
	virtual bool getValue(const std::string& name, uint1& result) const noexcept = 0;
	virtual bool getValue(const std::string& name, uint2& result) const noexcept = 0;
	virtual bool getValue(const std::string& name, uint3& result) const noexcept = 0;
	virtual bool getValue(const std::string& name, uint4& result) const noexcept = 0;
	virtual bool getValue(const std::string& name, float1& result) const noexcept = 0;
	virtual bool getValue(const std::string& name, float2& result) const noexcept = 0;
	virtual bool getValue(const std::string& name, float3& result) const noexcept = 0;
	virtual bool getValue(const std::string& name, float4& result) const noexcept = 0;
	virtual bool getValue(const std::string& name, std::string& result) const noexcept = 0;

	virtual void lock() noexcept;
	virtual void unlock() noexcept;

private:
	archivebuf(const archivebuf&) noexcept = delete;
	archivebuf& operator=(const archivebuf&) noexcept = delete;
};

_NAME_END

#endif