// David Hart - 2012

#pragma once

template <typename T> void Message::Append(const T& value)
{
	Append((char*)&value, sizeof(T));
}

template <> inline void Message::Append<std::string>(const std::string& string)
{
	Append<unsigned short>((short)string.size());
	Append(string.data(), string.size());
}

template <typename T> bool Message::Read(T& value)
{
	return Read(&value, sizeof(T));
}

template <> inline bool Message::Read<std::string>(std::string& value)
{
	unsigned short length;

	if (!Read(length))
		return false;

	return ReadString(value, length);
}