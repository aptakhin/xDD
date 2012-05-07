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

void Value::parse_str(const QString& str)
{
	bool ok = true;
	switch (type)
	{
	case T_BOOL:
		if (str == "+") v_bool = true;
		else if (str == "-") v_bool = false;
		else XDD_ERR("Can't parse");
		break;

	case T_UINT32:
		v_uint32 = str.toUInt(&ok);
		if (!ok) XDD_ERR("Can't parse");
		break;

	case T_UINT64:
		v_uint64 = str.toULongLong(&ok);
		if (!ok) XDD_ERR("Can't parse");
		break;

	default:
		not_implemented("Can't parse string `" + str + "` to type " + type);
	}
}

Setting::Setting(const QString& name, Value::Type type, I_love_settings* binding)
:	_name(name),
	_val(type),
	_parent(nullptr),
	_binding(binding)
{
}

Setting::Setting(Setting* group, const QString& name, Value::Type type)
:	_name(name),
	_val(type),
	_parent(group),
	_binding(nullptr)
{
	group->_settings.push_back(this);
}

void Setting::parse_str(const QString& str)
{
	_val.parse_str(str);
}

void Setting::exp(Value::Type t) const
{
	XDD_ASSERT2(t == _val.type, "Expected type: " << _val.type << ", but got " << t); 
}

void Setting::update_binded()
{
	XDD_ASSERT3(_parent != nullptr, "Expected parent", return);
	XDD_ASSERT3(_parent->_binding != nullptr, "Expected binding of parent", return);
	_parent->_binding->import_setting(this);
}

Settings_manager* Settings_manager::_instance = nullptr;

Settings_manager::Settings_manager()
:	_config("xDDConfig.yaml"),
	_export(""),
	_root("root", Value::T_GROUP, nullptr)
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
	Settings_manager::i()->_root._settings.push_back(group);
}

Setting* Settings_manager::find_setting(const QString& name)
{
	return _find_setting(&_root, name);
}

Setting* Settings_manager::_find_setting(Setting* setting, const QString& name)
{
	Setting::Settings::const_iterator i = setting->settings().begin();

	// One lambda-function dead here. RIP

	for (; i != setting->settings().end(); ++i)
	{
		Setting* s = *i;
		if (s->_name == name)
			return s;
		else if (!s->settings().empty())
			return _find_setting(s, name);			
	}
	return nullptr;
}

void Settings_manager::notify_everything_initialized()
{
	// For portable version. Config file near the binary has max priority.
	if (QDir::current().exists(_config))
	{
		_export = QDir::current().filePath(_config);
	}
	else 
	{
		// Otherwise use file in user home folder
		_export = QDir::home().filePath(_config);
		if (!QDir::home().exists(_config)) // Write default config
			write_config(_export);
	}

	XDD_LOG("Settings manager uses config file at: " << _export);
	read_config(_export);

	XDD_LOG("Settings manager initialized");
}

void Settings_manager::update()
{
	write_config(_export);
}

YAML::Emitter& operator << (YAML::Emitter& out, const QString& str)
{
	return out << qPrintable(str);
}

YAML::Emitter& operator << (YAML::Emitter& out, const Value& val)
{
	return out << val.to_str();
}

void Settings_manager::write_config_node(YAML::Emitter* emit_out, const Setting* node)
{
	YAML::Emitter& out = *emit_out;

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
			Settings_manager::write_config_node(&out, child);
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

	write_config_node(&out, &_root);

	std::ofstream fout(filename.toStdWString());
	fout << out.c_str();
}

void Settings_manager::read_config_node(Setting* setting, const YAML::Node* node)
{
	std::string key, value;
	for(YAML::Iterator i = node->begin(); i != node->end(); ++i) 
	{
		i.first() >> key;
		QString sub_setting_name = QString::fromStdString(key);

		if (Setting* sub_setting = _find_setting(setting, sub_setting_name))
		{
			if (sub_setting->settings().empty())
			{
				i.second() >> value;
				QString qvalue = QString::fromStdString(value);
				sub_setting->parse_str(qvalue);
				sub_setting->update_binded();
			}
			else
			{
				read_config_node(sub_setting, &i.second());
			}
		}
	}
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
				std::string key, value;
				i.first() >> key;

				QString setting_name = QString::fromStdString(key);

				if (Setting* setting = find_setting(setting_name))
				{
					if (setting->settings().empty())
					{
						i.second() >> value;
						QString qvalue = QString::fromStdString(value);
						setting->parse_str(qvalue);
						setting->update_binded();
					}
					else
					{
						read_config_node(setting, &i.second());
					}
				}
			}
		}
	} 
	catch(YAML::ParserException& e) 
	{
		std::cout << e.what() << "\n";
	}
}

}// namespace xdd
