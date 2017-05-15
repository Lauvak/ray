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
#include <ray/archive_buf.h>

_NAME_BEGIN

const archive_node& archive_node::nil = archive_node();
const archive_node& archive_node::nilRef = archive_node();

archivebuf::archivebuf() noexcept
{
}

archivebuf::~archivebuf() noexcept
{
}

void
archivebuf::lock() noexcept
{
}

void
archivebuf::unlock() noexcept
{
}

archive_node::archive_node()
{
}

archive_node::archive_node(type_t value)
{
	this->emplace(value);
}

archive_node::archive_node(boolean_t value)
	: _data(value)
{
}

archive_node::archive_node(number_integer_t value)
	: _data(value)
{
}

archive_node::archive_node(number_unsigned_t value)
	: _data(value)
{
}

archive_node::archive_node(number_float_t value)
	: _data(value)
{
}

archive_node::archive_node(string_t&& value)
	: _data(std::make_unique<string_t>(std::move(value)))
{
}

archive_node::archive_node(const string_t& value)
	: _data(std::make_unique<string_t>(value))
{
}

archive_node::archive_node(const string_t::value_type* value)
	: _data(std::make_unique<string_t>(value))
{
}

archive_node::archive_node(archive_node&& node)
	: _data(std::move(node._data))
{
}

archive_node::archive_node(const archive_node& node)
{
	switch (node.type())
	{
	case archive_node::type_t::null:
		_data.emplace<void*>();
		break;
	case archive_node::type_t::boolean:
		_data.emplace<boolean_t>(std::get<archive_node::type_t::boolean>(node._data));
		break;
	case archive_node::type_t::number_integer:
		_data.emplace<number_integer_t>(std::get<archive_node::type_t::number_integer>(node._data));
		break;
	case archive_node::type_t::number_unsigned:
		_data.emplace<number_unsigned_t>(std::get<archive_node::type_t::number_unsigned>(node._data));
		break;
	case archive_node::type_t::number_float:
		_data.emplace<number_float_t>(std::get<archive_node::type_t::number_float>(node._data));
		break;
	case archive_node::type_t::string:
		_data.emplace<std::unique_ptr<string_t>>(std::make_unique<string_t>(*std::get<archive_node::type_t::string>(node._data)));
		break;
	case archive_node::type_t::array:
		break;
	case archive_node::type_t::object:
		_data.emplace<std::unique_ptr<object_t>>(std::make_unique<object_t>(*std::get<archive_node::type_t::object>(node._data)));
		break;
	default:
		break;
	}
}

archive_node::~archive_node()
{
}

archive_node&
archive_node::at(const string_t& key)
{
	if (this->type() == archive_node::type_t::null)
		this->emplace(archive_node::type_t::object);

	auto& data = std::get<archive_node::type_t::object>(_data);

	for (auto& it : *data)
		if (it.first == key)
			return it.second;

	data->push_back(std::make_pair(key, null));
	return data->back().second;
}

archive_node&
archive_node::at(const string_t::value_type* key)
{
	if (this->type() == archive_node::type_t::null)
		this->emplace(archive_node::type_t::object);

	auto& data = std::get<archive_node::type_t::object>(_data);

	for (auto& it : *data)
		if (it.first == key)
			return it.second;

	data->push_back(std::make_pair(key, null));
	return data->back().second;
}

const archive_node&
archive_node::at(const string_t& key) const
{
	assert(this->type() == archive_node::type_t::object);

	if (this->type() == archive_node::type_t::object)
	{
		auto& data = std::get<archive_node::type_t::object>(_data);

		for (auto& it : *data)
			if (it.first == key)
				return it.second;
	}

	return archive_node::nil;
}

const archive_node&
archive_node::at(const string_t::value_type* key) const
{
	assert(this->type() == archive_node::type_t::object);

	if (this->type() == archive_node::type_t::object)
	{
		auto& data = std::get<archive_node::type_t::object>(_data);

		for (auto& it : *data)
			if (it.first == key)
				return it.second;
	}

	return archive_node::nil;
}

void
archive_node::push_back(const string_t& key, boolean_t value)
{
	if (this->type() != archive_node::type_t::object)
		this->emplace(archive_node::type_t::object);

	auto& data = std::get<archive_node::type_t::object>(_data);
	data->push_back(std::make_pair(key, value));
}

void
archive_node::push_back(const string_t& key, const number_integer_t& value)
{
	if (this->type() != archive_node::type_t::object)
		this->emplace(archive_node::type_t::object);

	auto& data = std::get<archive_node::type_t::object>(_data);
	data->push_back(std::make_pair(key, value));
}

void
archive_node::push_back(const string_t& key, const number_unsigned_t& value)
{
	if (this->type() != archive_node::type_t::object)
		this->emplace(archive_node::type_t::object);

	auto& data = std::get<archive_node::type_t::object>(_data);
	data->push_back(std::make_pair(key, value));
}

void
archive_node::push_back(const string_t& key, const number_float_t& value)
{
	if (this->type() != archive_node::type_t::object)
		this->emplace(archive_node::type_t::object);

	auto& data = std::get<archive_node::type_t::object>(_data);
	data->push_back(std::make_pair(key, value));
}

void
archive_node::push_back(const string_t& key, const string_t& value)
{
	if (this->type() != archive_node::type_t::object)
		this->emplace(archive_node::type_t::object);

	auto& data = std::get<archive_node::type_t::object>(_data);
	data->push_back(std::make_pair(key, value));
}

void
archive_node::push_back(const string_t& key, const string_t::value_type* value)
{
	if (this->type() != archive_node::type_t::object)
		this->emplace(archive_node::type_t::object);

	auto& data = std::get<archive_node::type_t::object>(_data);
	data->push_back(std::make_pair(key, value));
}

void
archive_node::push_back(const string_t& key, archive_node&& value)
{
	if (this->type() != archive_node::type_t::object)
		this->emplace(archive_node::type_t::object);

	auto& data = std::get<archive_node::type_t::object>(_data);
	data->push_back(std::make_pair(key, value));
}

archive_node::iterator
archive_node::begin() noexcept
{
	switch (this->type())
	{
	case archive_node::type_t::object:
		if (std::get<archive_node::type_t::object>(_data))
			return std::get<archive_node::type_t::object>(_data)->begin();
		break;
	default:
		break;
	}

	return archive_node::iterator();
}

archive_node::iterator
archive_node::end() noexcept
{
	switch (this->type())
	{
	case archive_node::type_t::object:
		if (std::get<archive_node::type_t::object>(_data))
			return std::get<archive_node::type_t::object>(_data)->end();
		break;
	default:
		break;
	}

	return archive_node::iterator();
}

archive_node::const_iterator
archive_node::begin() const noexcept
{
	switch (this->type())
	{
	case archive_node::type_t::object:
		if (std::get<archive_node::type_t::object>(_data))
			return std::get<archive_node::type_t::object>(_data)->begin();
		break;
	default:
		break;
	}

	return archive_node::iterator();
}

archive_node::const_iterator
archive_node::end() const noexcept
{
	switch (this->type())
	{
	case archive_node::type_t::object:
		if (std::get<archive_node::type_t::object>(_data))
			return std::get<archive_node::type_t::object>(_data)->end();
		break;
	default:
		break;
	}

	return archive_node::iterator();
}

archive_node::reverse_iterator
archive_node::rbegin() noexcept
{
	switch (this->type())
	{
	case archive_node::type_t::object:
		if (std::get<archive_node::type_t::object>(_data))
			return std::get<archive_node::type_t::object>(_data)->rbegin();
		break;
	default:
		break;
	}

	return archive_node::reverse_iterator();
}

archive_node::reverse_iterator
archive_node::rend() noexcept
{
	switch (this->type())
	{
	case archive_node::type_t::object:
		if (std::get<archive_node::type_t::object>(_data))
			return std::get<archive_node::type_t::object>(_data)->rend();
		break;
	default:
		break;
	}

	return archive_node::reverse_iterator();
}

archive_node::const_reverse_iterator
archive_node::rbegin() const noexcept
{
	switch (this->type())
	{
	case archive_node::type_t::object:
		if (std::get<archive_node::type_t::object>(_data))
			return std::get<archive_node::type_t::object>(_data)->rbegin();
		break;
	default:
		break;
	}

	return archive_node::const_reverse_iterator();
}

archive_node::const_reverse_iterator
archive_node::rend() const noexcept
{
	switch (this->type())
	{
	case archive_node::type_t::object:
		if (std::get<archive_node::type_t::object>(_data))
			return std::get<archive_node::type_t::object>(_data)->rend();
		break;
	default:
		break;
	}

	return archive_node::const_reverse_iterator();
}

archive_node&
archive_node::front() noexcept
{
	assert(this->type() == archive_node::type_t::object);
	return std::get<archive_node::type_t::object>(_data)->front().second;
}

const archive_node& 
archive_node::front() const noexcept
{
	assert(this->type() == archive_node::type_t::object);
	return std::get<archive_node::type_t::object>(_data)->front().second;
}

archive_node&
archive_node::back() noexcept
{
	assert(this->type() == archive_node::type_t::object);
	return std::get<archive_node::type_t::object>(_data)->back().second;
}

const archive_node& 
archive_node::back() const noexcept
{
	assert(this->type() == archive_node::type_t::object);
	return std::get<archive_node::type_t::object>(_data)->back().second;
}

archive_node::type_t
archive_node::type() const noexcept
{
	return (type_t)_data.index();
}

void
archive_node::emplace(type_t type) noexcept
{
	switch (type)
	{
	case archive_node::type_t::null:
		_data.emplace<void*>();
		break;
	case archive_node::type_t::boolean:
		_data.emplace<bool>(false);
		break;
	case archive_node::type_t::number_integer:
		_data.emplace<number_integer_t>(0);
		break;
	case archive_node::type_t::number_unsigned:
		_data.emplace<number_unsigned_t>(0);
		break;
	case archive_node::type_t::number_float:
		_data.emplace<number_float_t>(0);
		break;
	case archive_node::type_t::string:
		_data.emplace<std::unique_ptr<string_t>>(std::make_unique<string_t>());
		break;
	case archive_node::type_t::array:
		break;
	case archive_node::type_t::object:
		_data.emplace<std::unique_ptr<object_t>>(std::make_unique<object_t>());
		break;
	default:
		break;
	}
}

archive_node&
archive_node::operator=(boolean_t value)
{
	_data = value;
	return *this;
}

void 
archive_node::resize(std::size_t size)
{
	if (this->type() != archive_node::type_t::object)
		this->emplace(archive_node::type_t::object);

	std::get<archive_node::type_t::object>(_data)->resize(size);
}

archive_node&
archive_node::operator=(number_integer_t value)
{
	_data = value;
	return *this;
}

archive_node&
archive_node::operator=(number_unsigned_t value)
{
	_data = value;
	return *this;
}

archive_node&
archive_node::operator=(number_float_t value)
{
	_data = value;
	return *this;
}

archive_node&
archive_node::operator=(string_t&& value)
{
	_data = std::make_unique<string_t>(std::move(value));
	return *this;
}

archive_node&
archive_node::operator=(const string_t& value)
{
	_data = std::make_unique<string_t>(value);
	return *this;
}

archive_node&
archive_node::operator=(archive_node&& value)
{
	_data = std::move(value._data);
	return *this;
}

archive_node&
archive_node::operator[](const char* key)
{
	return this->at(key);
}

archive_node&
archive_node::operator[](const string_t& key)
{
	return this->at(key);
}

const archive_node&
archive_node::operator[](const char* key) const
{
	return this->at(key);
}

const archive_node&
archive_node::operator[](const string_t& key) const
{
	return this->at(key);
}

_NAME_END