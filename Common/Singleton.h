#ifndef SINGLETON_H
#define SINGLETON_H

template<class T>
class Singleton
{
public:

public:
	static T * GetInstance()
	{
		static T t;
		return &t;
	}

protected:
	Singleton()
	{

	}
};

#endif