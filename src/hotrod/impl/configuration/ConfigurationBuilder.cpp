

#include "hotrod/impl/configuration/ConfigurationBuilder.h"
#include <sstream>
#include <fstream>
#include <iostream>

namespace infinispan {
namespace hotrod {

const char* ConfigurationBuilder::PROTOCOL_VERSION_12 = "1.2";

ConfigurationBuilder::ConfigurationBuilder()
: internalPingOnStartup(true), internalTcpNoDelay(true)
{
  ConfigurationBuilder::connectionPoolConfigurationBuilder = ConnectionPoolConfigurationBuilder();
  ConfigurationBuilder::sslConfigurationBuilder = SslConfigurationBuilder();
}

ServerConfigurationBuilder& ConfigurationBuilder::addServer()
{
  ServerConfigurationBuilder* builder = new ServerConfigurationBuilder();
  ConfigurationBuilder::internalServers.push_back(builder);
  return *builder;
}

ConfigurationBuilder& ConfigurationBuilder::addServers(std::string servers)
{
  std::vector<std::string> splittedServers;
  std::istringstream originalStream(servers);
  std::string stringHelper;
  std::string hostHelper;
  std::string portHelper;
  int portInt;

  while(std::getline(originalStream,stringHelper,';')) {

      std::istringstream singleServerStream(stringHelper);
      std::getline(singleServerStream,hostHelper,':');
      std::getline(singleServerStream,portHelper,':');
      std::istringstream portStream(portHelper);
      portStream >> portInt;
      this->addServer().host(hostHelper).port(portInt);
  }

  return *this;
}

ConnectionPoolConfigurationBuilder& ConfigurationBuilder::connectionPool()
{
  return ConfigurationBuilder::connectionPoolConfigurationBuilder;
}

ConfigurationBuilder& ConfigurationBuilder::connectionTiemout(int connectionTimeoutParam)
{
  ConfigurationBuilder::internalConnectionTimeout = connectionTimeoutParam;
  return *this;
}

ConfigurationBuilder& ConfigurationBuilder::forceReturnValues(bool forceReturnValuesParam)
{
  ConfigurationBuilder::internalForceReturnValue = forceReturnValuesParam;
  return *this;
}

ConfigurationBuilder& ConfigurationBuilder::keySizeEstimate(int keySizeEstimateParam)
{
  ConfigurationBuilder::internalKeySizeEstimate = keySizeEstimateParam;
  return *this;
}

ConfigurationBuilder& ConfigurationBuilder::pingOnStartup(bool pingOnStartupParam)
{
  ConfigurationBuilder::internalPingOnStartup = pingOnStartupParam;
  return *this;
}

ConfigurationBuilder& ConfigurationBuilder::protocolVersion(std::string protocolVersionParam)
{
  ConfigurationBuilder::internalProtocolVersion = protocolVersionParam;
  return *this;
}

ConfigurationBuilder& ConfigurationBuilder::socketTimeout(int socketTimeoutParam)
{
  ConfigurationBuilder::internalSocketTimeout = socketTimeoutParam;
  return *this;
}

SslConfigurationBuilder& ConfigurationBuilder::ssl()
{
  return ConfigurationBuilder::sslConfigurationBuilder;
}

ConfigurationBuilder& ConfigurationBuilder::tcpNoDelay(bool tcpNoDelayParam)
{
  ConfigurationBuilder::internalTcpNoDelay = tcpNoDelayParam;
  return *this;
}

ConfigurationBuilder& ConfigurationBuilder::valueSizeEstimate(int valueSizeEstimateParam)
{
  ConfigurationBuilder::internalValueSizeEstimate = valueSizeEstimateParam;
  return *this;
}

Configuration ConfigurationBuilder::build()
{
  return ConfigurationBuilder::create();
}

Configuration ConfigurationBuilder::create()
{
  std::vector<ServerConfiguration> servers;
  if(this->internalServers.size() > 0) {
      for(int i = 0; i < this->internalServers.size(); i++) {
          servers.push_back(internalServers[i]->create());
      }
  } else {
      servers.push_back(ServerConfiguration("127.0.0.1",11222));
  }

  //TODO: subsitute connectionpoolconfiguration and sslconfiguration with the builders
  return Configuration(this->internalProtocolVersion,
	      ConnectionPoolConfiguration(
	    		  WAIT,
	    		  true,
	    		  -1,
	    		  -1,
	    		  -1,
	    		  -1,
	    		  -1,
	    		  120000,
	    		  1800000,
	    		  3,
	    		  false,
	    		  false,
	    		  true),
      this->internalSocketTimeout,
      this->internalForceReturnValue,
      this->internalKeySizeEstimate,
      this->internalPingOnStartup,
      servers,
      this->internalSocketTimeout,
      SslConfiguration(),
      this->internalTcpNoDelay,
      this->internalKeySizeEstimate);
}

ConfigurationBuilder& ConfigurationBuilder::read(Configuration& bean)
{
  //TODO: implements the method
  return *this;
}


}} /* namespace */
