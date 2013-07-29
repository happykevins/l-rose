#ifndef LROS_DEFINES_H_
#define LROS_DEFINES_H_

#include <list>
#include <vector>
#include <map>
#include <functional>
#include <type_traits>
#include <iostream>
#include <cassert>

namespace lros
{

// Max Fields Count
static const int kMaxFieldCount = 0xff;
// Max Field ID NO.
static const int kMaxFiledIDNum = 0xff;

//
// LRoleType - indicate the remote role
//
enum LRoleType
{
	kRole_HOST,
	kRole_SLAVE,
};

//
// LRepType
//	- indicate how to replicate objects
//
enum LRepType
{
	kRep_Never,		// never replicate
	kRep_Create,	// replicate only when create, then will tear-off from host
	kRep_Normal,	// replicate when dirty
	kRep_Order		// replicate when dirty, and must make sure the time sequence
};

//
// LTypeID
//	
//
enum LTypeID
{
	l_void	= 0,	// None Type or Class
	l_root	= 0,	// LObject Root Class
	l_bool	= 1,
	l_byte	= 2,
	l_int16	= 3,
	l_int32	= 4,
	l_int64	= 5,
	l_float = 6,
	l_double= 7,
//	l_fix32	= 8,
//	l_fix64 = 9,
	l_string= 0x0a,	// std::string
//	l_binary= 0x0b,
//	l_enum	= 0x0c,
//	l_meta	= 0x0d,	// meta-data
	l_serv	= 0x0e,	// RPC service
	kTypeIDBasicMax	=0x0f,
	kTypeIDUserMax	=0xffff,
};

typedef bool lbool;
typedef char lbyte;
typedef short lint16;
typedef int lint32;
typedef long long lint64;
typedef float lfloat;
typedef double ldouble;
typedef std::string lstring;

template<typename T> struct LTypeTrait { static const int kValue = l_void; };
template<> struct LTypeTrait<lbool> { static const int kValue = l_bool; };
template<> struct LTypeTrait<lbyte> { static const int kValue = l_byte; };
template<> struct LTypeTrait<lint16> { static const int kValue = l_int16; };
template<> struct LTypeTrait<lint32> { static const int kValue = l_int32; };
template<> struct LTypeTrait<lint64> { static const int kValue = l_int64; };
template<> struct LTypeTrait<lfloat> { static const int kValue = l_float; };
template<> struct LTypeTrait<ldouble> { static const int kValue = l_double; };
//template<> struct LType<fix32> { static const int kValue = l_fix32; };
//template<> struct LType<fix64> { static const int kValue = l_fix64; };
template<> struct LTypeTrait<lstring> { static const int kValue = l_string; };
//template<> struct LType<byte*> { static const int kValue = l_binary; };


template<typename T>
class LRef
{
public:
	LRef()
		: m_object(NULL)
	{
	}

	LRef(T* object)
		: m_object(object)
	{
	}
	LRef(const LRef<T>& rhs)
		: m_object(rhs.m_object)
	{
	}
	LRef<T>& operator=(const LRef<T>& rhs)
	{
		m_object = rhs.m_object;
		return *this;
	}

	void set_ref_id(int _id)
	{

	}

	int ref_id() const 
	{
		return 0;
	}

	T* get()
	{
		return m_object;
	}

	const T* get() const
	{
		return m_object;
	}

	T* operator->()
	{
		return m_object;
	}

	const T* operator->() const
	{
		return m_object;
	}

	T& operator*()
	{
		return *m_object;
	}

	const T& operator*() const
	{
		return *m_object;
	}

private:
	T* m_object;
};


template<typename T>
class LMember
{
private:
	LMember(T* object)
		: m_object(object)
	{
	}
	LMember(const LMember<T>& rhs)
		: m_object(rhs.m_object)
	{
	}
	LMember<T>& operator=(const LMember<T>& rhs)
	{
		m_object = rhs.m_object;
		return *this;
	}

public:
	LMember()
		: m_object(NULL)
	{
		m_object = new T;
		assert(m_object);
	}

	~LMember()
	{
		assert(m_object);
		delete m_object;
		m_object = NULL;
	}

	void set_id(int _id)
	{

	}

	int id() const 
	{
		return 0;
	}

	T* get()
	{
		return m_object;
	}

	const T* get() const
	{
		return m_object;
	}

	T* operator->()
	{
		return m_object;
	}

	const T* operator->() const
	{
		return m_object;
	}

	T& operator*()
	{
		return *m_object;
	}

	const T& operator*() const
	{
		return *m_object;
	}

private:
	T* m_object;
};

class LObject;
class LStream;

// Meta Class Info
class LClass
{
public:
	typedef std::function<bool(LStream&, const LObject*)> Serializer;
	typedef std::function<bool(LStream&, LObject*)> Deserializer;
	typedef std::function<LObject*()> Creater;
	typedef std::function<void()> Initializer;

	LClass(int class_id, const char* class_name, const LClass* super_class,
		Creater creater, Serializer serializer, Deserializer deserializer, Initializer initializer);

	inline int class_id() const { return m_class_id; }
	inline const char* class_name() const { return m_class_name; }
	inline const LClass* super_class() const { return m_super_class; }

	inline LObject* create_object() { return m_creater(); }
	inline const Serializer& serializer() const { return m_serializer; }
	inline const Deserializer& deserializer() const { return m_deserializer; }

	static const LClass* class_for(int class_id);
	static LObject* create_object(const LClass* cls);
	static LObject* create_object(int class_id);

private:
	static std::map<int, const LClass*> s_class_map;
	int m_class_id;
	const char* m_class_name;
	const LClass* m_super_class;
	Creater m_creater;
	Serializer m_serializer;
	Deserializer m_deserializer;
};


};//lros

#endif //LROS_DEFINES_H_