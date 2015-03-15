// Aggregates all information needed to start and monitor nodes
// Author: Max Schwarz <max.schwarz@uni-bonn.de>

#ifndef LAUNCH_CONFIG_H
#define LAUNCH_CONFIG_H

#include "node.h"
#include "fd_watcher.h"

#include <map>
#include <vector>
#include <stdexcept>

#include <XmlRpc.h>
#include <tinyxml.h>
#include <yaml-cpp/yaml.h>

#include <ros/node_handle.h>

#include <boost/signals2.hpp>

namespace rosmon
{

class LaunchConfig
{
public:
	class ParseException : public std::exception
	{
	public:
		explicit ParseException(const std::string& msg)
		 : m_msg(msg)
		{}

		virtual ~ParseException() throw()
		{}

		virtual const char* what() const noexcept
		{ return m_msg.c_str(); }
	private:
		std::string m_msg;
	};

	explicit LaunchConfig(const FDWatcher::Ptr& watcher);
	~LaunchConfig();

	void parse(const std::string& filename);

	void setParameters();
	void start();
	void shutdown();
	void forceExit();
	bool allShutdown();

	inline const std::vector<Node::Ptr>& nodes() const
	{ return m_nodes; }

	boost::signals2::signal<void(std::string,std::string)> logMessageSignal;
private:
	class ParseContext
	{
	public:
		ParseContext(const std::string& filename)
		 : m_prefix("/")
		 , m_filename(filename)
		{}

		const std::string& prefix() const
		{ return m_prefix; }

		const std::string& filename() const
		{ return m_filename; }

		void setFilename(const std::string& filename)
		{ m_filename = filename; }

		ParseContext enterScope(const std::string& prefix)
		{
			ParseContext ret = *this;
			ret.m_prefix = ret.m_prefix + prefix;
			if(prefix[prefix.size()-1] != '/')
				ret.m_prefix.push_back('/');

			return ret;
		}

		std::string evaluate(const std::string& tpl);

		void clearArguments()
		{
			m_args.clear();
		}

		void setArg(const std::string& name, const std::string& argument, bool override);

		bool shouldSkip(TiXmlElement* element);
	private:
		std::string m_prefix;
		std::string m_filename;
		std::map<std::string, std::string> m_args;
	};

	void parse(TiXmlElement* element, ParseContext context);
	void parseNode(TiXmlElement* element, ParseContext context);
	void parseParam(TiXmlElement* element, ParseContext context);
	void parseROSParam(TiXmlElement* element, ParseContext context);
	void parseInclude(TiXmlElement* element, ParseContext context);
	void parseArgument(TiXmlElement* element, ParseContext& context);

	void loadYAMLParams(const YAML::Node& n, const std::string& prefix);

	XmlRpc::XmlRpcValue yamlToXmlRpc(const YAML::Node& n);

	ros::NodeHandle m_nh;
	FDWatcher::Ptr m_fdWatcher;

	std::vector<Node::Ptr> m_nodes;
	std::map<std::string, XmlRpc::XmlRpcValue> m_params;

	void log(const char* fmt, ...) __attribute__((format (printf, 2, 3)));
};

}

#endif

