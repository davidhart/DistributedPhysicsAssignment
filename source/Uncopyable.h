// David Hart - 2012

#pragma once

class Uncopyable
{

protected:

	Uncopyable();

private: 

	Uncopyable(const Uncopyable&);
	Uncopyable& operator=(const Uncopyable&);

};
