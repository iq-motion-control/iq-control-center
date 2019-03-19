/*
    Copyright 2018 - 2019 IQ Motion Control   	dskart11@gmail.com

    This file is part of IQ Control Center.

    IQ Control Center is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    IQ Control Center is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef LOG_HPP
#define LOG_HPP


#include "json.h"
#include "json-forwards.h"

#include <iostream>
#include <memory>

class JsonCpp{
	private:

	public:
		Json::StreamWriterBuilder wbuilder;
		Json::CharReaderBuilder rbuilder;
		std::unique_ptr<Json::StreamWriter> writer;

		JsonCpp();
};

#endif
