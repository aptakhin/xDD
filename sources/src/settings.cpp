/** xDDTools */
#include "xdd/settings.hpp"
#include <QDir>
#include <yaml-cpp/yaml.h>

namespace xdd {

Value::Value(Type type) 
:	type(type),
	v_bool(false),
	v_uint32(0),
	v_uint64(0)
{}

QString Value::to_str() const
{
	switch (type)
	{
	case T_BOOL:
		return v_bool? QString("+") : QString("-");
	case T_UINT32:
		return QString::number(v_uint32);
	case T_UINT64:
		return QString::number(v_uint64);
	default:
		not_implemented("Value to type: " + type);
		return "";
	}
}

Setting::Setting(const QString& name, Value::Type type)
:	_name(name),
	_val(type)
{
}

Setting::Setting(Setting* group, const QString& name, Value::Type type)
:	_name(name),
	_val(type)
{
	group->_settings.push_back(this);
}

void Setting::exp(Value::Type t) const
{
	XDD_ASSERT2(t == _val.type, "Expected type: " << _val.type << ", but got " << t); 
}

Settings_manager* Settings_manager::_instance = nullptr;

Settings_manager::Settings_manager()
:	_config("xDDConfig.yaml"),
	_export(""),
	_root("root", Value::T_GROUP)
{
	XDD_ASSERT3(!Settings_manager::_instance,
		"Singleton of Settings_manager is already created!",
			return);
	Settings_manager::_instance = this;
	XDD_LOG("Settings manager startup");
}

Settings_manager::~Settings_manager()
{
	XDD_ASSERT3(Settings_manager::_instance == this,
		"Problem while deleting Settings_manager! Another singleton was created!",
			return);
	XDD_LOG("Settings manager destroyed");

	Settings_manager::_instance = 0;
}

void Settings_manager::bind_group(Setting* group)
{
	bind_in_group(group, &Settings_manager::i()->_root);
}

void Settings_manager::bind_in_group(Setting* setting, Setting* group)
{
	group->_settings.push_back(setting);
}

Setting* Settings_manager::find_setting(const QString& name)
{
	return _find_setting(&_root, name);
}

Setting* Settings_manager::_find_setting(Setting* setting, const QString& name)
{
	Setting::Settings::const_iterator i = setting->settings().begin();

	for (; i != setting->settings().end(); ++i)
	{
		Setting* s = *i;
		if (s->_name == name)
			return s;
		else if (!s->settings().empty())
			return this->_find_setting(s, name);
		else
			return nullptr;
	}
}

void Settings_manager::notify_everything_initialized()
{
	if (QDir::current().exists(_config))
	{
		_export = QDir::current().filePath(_config);
	}
	else 
	{
		_export = QDir::home().filePath(_config);
		if (!QDir::home().exists(_config))
			write_config(_export);
	}
	write_config(_export);

	XDD_LOG("Settings manager uses config file at: " << _export);
	read_config(_export);

	XDD_LOG("Settings manager initialized");
}

YAML::Emitter& operator << (YAML::Emitter& out, const QString& str)
{
	return out << qPrintable(str);
}

YAML::Emitter& operator << (YAML::Emitter& out, const Value& val)
{
	QString q = val.to_str();
	return out << val.to_str();
}

// Not a member of Settings_manager due to source internal YAML including
void write_config_node(YAML::Emitter& out, const Setting* node)
{
	out << YAML::BeginMap;

	fun::each(node->settings(), [&out] (const Setting* child) {
		if (child->settings().empty())
		{
			out << YAML::Key   << child->name();
			out << YAML::Value << child->value();
		}
		else
		{
			out << YAML::Key << child->name();
			out << YAML::Value;
			write_config_node(out, child);
		}
	});

	out << YAML::EndMap;
}

void Settings_manager::write_config(const QString& filename)
{
	YAML::Emitter out;
	out << YAML::Comment("Default xDD config");

	out << YAML::BeginMap;
	out << YAML::Key << "version" << YAML::Value << 1;
	out << YAML::EndMap;

	write_config_node(out, &_root);

	std::ofstream fout(filename.toStdWString());
	fout << out.c_str();
}

void Settings_manager::read_config(const QString& filename)
{
	std::ifstream fin(filename.toStdWString());

	try
	{
		YAML::Parser parser(fin);

		YAML::Node doc;
		while(parser.GetNextDocument(doc))
		{
			Setting* group = &_root;

			for(YAML::Iterator i = doc.begin(); i != doc.end(); ++i) 
			{
			
			}
		}
	} 
	catch(YAML::ParserException& e) 
	{
		std::cout << e.what() << "\n";
	}
}

}// namespace xdd
