/** xDDTools */
#pragma once

#include "xdd/common.hpp"

namespace xdd {
/*
class Setting
{
public:

	enum Type
	{
		T_BOOL,
		T_UINT32,
	};

protected:
	QString _name;
	uint32 _id;
	Type _type;
};

class Setting_value
{
public:
	struct Value
	{
		union {
			bool b;
			uint32 ui32;
		};
		uint32 size;
	};

	enum Type
	{
		T_BOOL,
		T_UINT32,
	};

protected:
	uint32 _sett_id;
	Value _value;
};


class bind_setting;

class Settings_manager
{
public:

	struct Setting
	{

	};

	static Settings_manager* i() { return _instance; }

protected:

	void bind(const QString& name, Setting::Type type, void* var);

	static Settings_manager* _instance;

	friend class bind_setting;
};

class bind_setting
{
public:

	explicit bind_setting(const QString& name, bool* var, const std::function<bool>& func_on_update = std::function<bool>())
	{
		Settings_manager::i()->bind(name, &var)
	}

	explicit bind_setting(const QString& name, uint32* var, const std::function<uint32>& func_on_update = std::function<uint32>());
};
*/
}// namespace xdd
