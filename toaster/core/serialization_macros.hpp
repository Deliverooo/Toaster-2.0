#pragma once

#define TST_SERIALIZE_PROPERTY(propName, propVal, outputNode) outputNode << YAML::Key << #propName << YAML::Value << propVal

#define TST_DESERIALIZE_PROPERTY(propertyName, destination, node, defaultValue)	\
if (node.IsMap())																\
{																				\
if (auto foundNode = node[#propertyName])									\
{																			\
try																		\
{																		\
destination = foundNode.as<decltype(defaultValue)>();				\
}																		\
catch (const std::exception& e)											\
{																		\
TST_ERROR("{}", e.what());										\
\
destination = defaultValue;											\
}																		\
}																			\
else																		\
{																			\
destination = defaultValue;												\
}																			\
}																				\
else																			\
{																				\
destination = defaultValue;													\
}
