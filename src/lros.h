//
// LROS - lite remote object system
//	author: kevin sun <happykevins@gmail.com> 
//
#ifndef LROS_ROS_H_
#define LROS_ROS_H_

#include "lobject.h"
#include "lstream.h"

//
// LROS Types
//	1.Object Entity
//	2.LROS Reference
//	3.Meta Data
//
//	TODO:
//	- default value
//	- array support, access, replicate
//	- treat member as ref when replicate, combine member & ref class
//	- ensure LClass class map init first, init static member use new
//

//
// Serialization
//	* TODO List: !!!! 重新设计
//	- type define: op_type, type_id(class_id), obj_id, slot_id, slot_value(data)
//	- replicate pack: # obj_id # [ [slot_id # slot_value] | [(ref)slot_id # (ref)obj_id # [slot_id # slot_value]... ]... #
//	- when create	: # (op_type)op_create # type_id # replicate pack #
//	- when destroy	: # (op_type)op_destroy # obj_id #
//	- when replicate: # (op_type)op_replicate # obj_id # replicate pack #
//	- ??rpc service	: # (op_type)op_rpc # ??
//

//
// Meta-data - data never changed, and reads from the local template config files
//	* TODO List:
//	- meta-data system
//	- initialized when create
//

namespace lros
{

// dump class info
template<typename C>
inline void _lros_dump_class_info()
{
	std::cout << "===== [" << C::l_meta_class()->class_name() << "] Info =====" << std::endl;
	for (auto f: C::LDerivedType::__field_registry.field_list)
	{
		std::cout << f.field_id << "\t" << f.field_name << std::endl;
	}
	std::cout << "----- Total: " << C::LDerivedType::__field_registry.field_list.size() << " field(s) -----" << std::endl;
}

#define LCLASS_IMPLEMENT(class_id, class_name) \
	lros::LFieldRegistry<class_name> class_name::LDerivedType::__field_registry; \
	const lros::LClass class_name::LDerivedType::__meta_class( \
		class_id, #class_name, class_name::LSuperClassType::l_meta_class(), \
		std::bind( \
		&class_name::LDerivedType::l_create_object), \
		std::bind( \
		&class_name::LDerivedType::l_serialize, \
		std::placeholders::_1, \
		std::placeholders::_2), \
		std::bind( \
		&class_name::LDerivedType::l_deserialize, \
		std::placeholders::_1, \
		std::placeholders::_2), \
		std::bind( \
		&class_name::LDerivedType::l_static_init) \
		); \


//////////////////////////////////////////////////////////////////////////
#define L_FIELD_LIST_BEGIN \
public: \
	template<typename T>	\
	static void __register_fields_impl() \
	{ \
		LDerivedType::__field_registry.fields_reserved = __get_super_fieldid_reserved(); \

#define L_FIELD_LIST_BEGIN_WITH_RESERVED(reserved_maxid) \
	L_FIELD_LIST_BEGIN \
	static_assert(reserved_maxid >= 0 && reserved_maxid < lros::kMaxFiledIDNum, "reserved Fileds ID must < kMaxFiledIDNum!"); \
	assert(reserved_maxid >= __get_super_fieldid_reserved() && "reserved Fileds ID must > Super Class reserved Fields ID!"); \
	LDerivedType::__field_registry.fields_reserved = reserved_maxid; \

#define L_FIELD_LIST_END \
	} \

#define L_REGISTER_FIELD(id, name) \
{ \
	static_assert(id >= 0 && id < lros::kMaxFiledIDNum, "Filed ID must < kMaxFiledIDNum !"); \
	assert(id > __get_super_fieldid_reserved() && "Filed ID must > Super Class reserved Fields ID!"); \
	lros::LField<T> _field; \
	_field.field_id = id; \
	_field.field_name = #name; \
	_field.serializer = std::bind( \
	&T::__serialize_##name, \
	std::placeholders::_1,  \
	std::placeholders::_2); \
	_field.deserializer = std::bind( \
	&T::__deserialize_##name, \
	std::placeholders::_1,  \
	std::placeholders::_2); \
	T::LDerivedType::__field_registry.field_list.push_back(_field); \
	assert(T::LDerivedType::__field_registry.field_list.size() < lros::kMaxFieldCount \
		&& "LROS class fields count must < kMaxFieldCount!"); \
	assert(T::LDerivedType::__field_registry.field_map.find(id) \
		==  T::LDerivedType::__field_registry.field_map.end() \
		&& "Field id conflict, check if id is already used in this or super class!"); \
	T::LDerivedType::__field_registry.field_map[id] = _field; \
} \

#define L_FIELD_STD(name, type) \
private:	\
	type __##name; \
public:		\
	type get_##name() const { return __##name; } \
	void set_##name(const type& ##name) { __##name = ##name; } \
	static bool __serialize_##name(lros::LStream& s, const LClassType& e) \
	{ \
		static_assert(lros::LTypeTrait<type>::kValue!=lros::l_void, "Invalid std type, see LType list for supported std types!"); \
		std::cout << __FUNCTION__ << std::endl; \
		return s.write(e.__##name); \
	} \
	static bool __deserialize_##name(lros::LStream& s, LClassType& e) \
	{ \
		std::cout << __FUNCTION__ << std::endl; \
		return s.read(e.__##name); \
	} \

#define __L_FIELD_REF(name, ref_type, raw_type) \
private:	\
	ref_type __##name; \
public:		\
	raw_type* get_##name() { return __##name.get(); } \
	void set_##name(raw_type* ##name) { __##name = ref_type(##name); } \
	static bool __serialize_##name(lros::LStream& s, const LClassType& e) \
	{ \
		std::cout << __FUNCTION__ << std::endl; \
		return s.write(e.__##name); \
	} \
	static bool __deserialize_##name(lros::LStream& s, LClassType& e) \
	{ \
		std::cout << __FUNCTION__ << std::endl; \
		return s.read(e.__##name); \
	} \
	
#define L_FIELD_REF(name, ref_type) \
	__L_FIELD_REF(name, lros::LRef<ref_type>, ref_type) \

#define __L_FIELD_MEM(name, member_type, raw_type) \
private:	\
	member_type __##name; \
public:		\
	raw_type& get_##name() { return *__##name; } \
	const raw_type& get_##name() const { return *__##name; } \
	static bool __serialize_##name(lros::LStream& s, const LClassType& e) \
	{ \
		std::cout << __FUNCTION__ << std::endl; \
		return s.write(e.__##name); \
	} \
	static bool __deserialize_##name(lros::LStream& s, LClassType& e) \
	{ \
		std::cout << __FUNCTION__ << std::endl; \
		return s.read(e.__##name); \
	} \

#define L_FIELD_MEM(name, member_type) \
	__L_FIELD_MEM(name, lros::LMember<member_type>, member_type) \

};

#endif //LROS_ROS_H_
