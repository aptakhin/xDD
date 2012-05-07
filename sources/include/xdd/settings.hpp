/** xDDTools */
#pragma once

#include "xdd/common.hpp"

namespace YAML {
	class Node;
	class Emitter;
}

namespace xdd {

class Setting;
class Settings_manager;

class I_love_settings
{
public:
	virtual void import_setting(Setting* setting) = 0;
};

struct Value
{
	enum Type
	{
		T_BOOL,
		T_UINT32,
		T_UINT64,
		T_GROUP
	};

	Value(Type type);

	Type type;

	QString to_str() const;
	void parse_str(const QString& str);

	bool v_bool;
	uint32 v_uint32;
	uint64 v_uint64;
};

class Setting
{
public:

	typedef std::vector<Setting*> Settings;

	Setting(Setting* group, const QString& name, Value::Type type);
	Setting(const QString& name, Value::Type type, I_love_settings* binding);

	virtual ~Setting() {}

	void exp(Value::Type t) const;

	bool is(const QString& name) const { return _name == name; }

	const QString& name() const { return _name; }
	const Value& value() const { return _val; }
	Value::Type type() const { return _val.type; }

#define GS(enum_type, ctype) void XDD_CAT(set_, ctype)(ctype v) { exp(Value::enum_type); _val.XDD_CAT(v_, ctype) = v; }    ctype XDD_CAT(get_, ctype)() const  { exp(Value::enum_type); return _val.XDD_CAT(v_, ctype); }

	GS(T_BOOL,   bool);
	GS(T_UINT32, uint32);
	GS(T_UINT64, uint64);

#undef GS

	const Settings& settings() const { return _settings; }

	void parse_str(const QString& str);

protected:
	QString _name;
	Value _val;

	I_love_settings* _binding;
	
	Settings _settings;

	friend class Settings_manager;
};

class Settings_manager
{
public:
	Settings_manager();
	~Settings_manager();

	static Settings_manager* i() { return _instance; }

	static void bind_group(Setting* group);

	void update();

	void notify_everything_initialized();

protected:

	void read_config(const QString& filename);
	static void read_config_node(Setting* setting, const YAML::Node* node);

	void write_config(const QString& filename);
	static void write_config_node(YAML::Emitter* emit_out, const Setting* node);

	Setting* find_setting(const QString& name);

	static Setting* _find_setting(Setting* setting, const QString& name);

protected:

	static Settings_manager* _instance;

	Setting _root;

	const QString _config;
	QString _export;
};

}// namespace xdd
