#include "lobject.h"

namespace lros {

std::map<int, const LClass*> LClass::s_class_map;

const LClass LObject::s_meta_class(
	(int)l_root, "LObject", NULL,
	std::bind(
	&LObject::l_create_object),
	std::bind(
	&LObject::l_serialize,
	std::placeholders::_1,
	std::placeholders::_2),
	std::bind(
	&LObject::l_deserialize,
	std::placeholders::_1,
	std::placeholders::_2),
	std::bind(
	&LObject::l_static_init)
	);

const LClass* LObject::l_class() const
{
	return &LObject::s_meta_class;
}

bool LObject::l_same_class(const LClass* cls) const
{
	return l_class()->class_id() == cls->class_id();
}

bool LObject::l_same_class(const LObject* obj) const
{
	return l_same_class(obj->l_class());
}

bool LObject::l_instance_of(const LClass* cls) const
{
	assert(cls && l_class());

	const LClass* this_class = cls; 
	const LClass* check_class = l_class();
	do
	{
		if (this_class->class_id() == check_class->class_id())
			return true;
		check_class = check_class->super_class();
	} while (check_class);

	return false;
}

bool LObject::l_instance_of(const LObject* obj) const
{
	return l_instance_of(obj->l_class());
}

bool LObject::l_serialize(lros::LStream& s, const LObject* obj)
{
	assert(0);
	return false;
}

bool LObject::l_deserialize(lros::LStream& s, LObject* obj)
{
	assert(0);
	return false;
}

}//lros