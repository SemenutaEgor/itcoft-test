#include <iostream>
#include <pugixml.hpp>
#include <vector>
#include <string>
#include <unordered_map>

void validate(const pugi::xml_document &doc)
{
    auto root = doc.child("ControlSystem");
    if (root.empty())
    {
        throw std::runtime_error("XML error: <ControlSystem> node missing");
    }

    auto ports = root.child("Ports");
    if (ports.empty())
    {
        throw std::runtime_error("XML error: <Ports> node missing");
    }

    auto port_list = ports.children("Port");
    if (port_list.begin() == port_list.end())
    {
        throw std::runtime_error("XML error: no <Port> entries inside <Ports>");
    }

    for (const auto &port : port_list)
    {
        if (port.attribute("ID").empty())
        {
            throw std::runtime_error("XML error: <Port> is missing ID attribute");
        }

        const std::vector<std::string> required_tags = {
            "Up",
            "AutoNegotiationEnabled",
            "AllowedSpeed_10M",
            "AllowedSpeed_100M",
            "AllowedSpeed_1G",
            "AllowedSpeed_10G"};

        for (const auto &name : required_tags)
        {
            if (port.child(name.c_str()).empty())
            {
                throw std::runtime_error("XML error: <Port> missing tag <" + name + ">");
            }
        }
    }

    auto vlans = root.child("Vlans");
    if (vlans.empty())
    {
        throw std::runtime_error("XML error: <Vlans> node missing");
    }

    auto def_vlans = vlans.child("defVlans");
    if (def_vlans.empty())
    {
        throw std::runtime_error("XML error: <defVlans> node missing");
    }

    auto def_port_list = def_vlans.children("port");
    if (def_port_list.begin() == def_port_list.end())
    {
        throw std::runtime_error("XML error: no <port> entries inside <defVlans>");
    }

    for (const auto &def_port : def_port_list)
    {
        if (def_port.attribute("ID").empty())
        {
            throw std::runtime_error("XML error: <Port> is missing ID attribute");
        }

        auto vlan_node = def_port.child("vlan");
        if (vlan_node.empty())
        {
            throw std::runtime_error("XML error: <vlan> tag missing");
        }

        std::string vlan_value = vlan_node.child_value();
        if (vlan_value.empty())
        {
            throw std::runtime_error("XML error: <vlan> tag has no value");
        }
    }
}

int main()
{
    pugi::xml_document doc;

    if (doc.load_file("../../test_config.xml").status != pugi::xml_parse_status::status_ok)
    {
        std::cerr << "Error: Configuration file not found" << std::endl;
        return 1;
    }

    try
    {
        validate(doc);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    auto ports = doc.child("ControlSystem").child("Ports");
    auto def_vlans = doc.child("ControlSystem").child("Vlans").child("defVlans");
    std::unordered_map<int, int> default_vlan;
    for (auto &defp : def_vlans.children("port"))
    {
        int id = defp.attribute("ID").as_int();
        int vlan = defp.child("vlan").text().as_int();
        default_vlan[id] = vlan;
    }

    for (auto &port : ports.children("Port"))
    {
        const auto id = port.attribute("ID").as_int();
        std::cout << "Port ID=\"" << id << "\"" << std::endl;
        std::cout << "  Up: " << port.child("Up").child_value() << std::endl;
        std::cout << "  AutoNegotiationEnabled: " << port.child("AutoNegotiationEnabled").child_value() << std::endl;
        std::cout << "  AllowedSpeed_10M: " << port.child("AllowedSpeed_10M").child_value() << std::endl;
        std::cout << "  AllowedSpeed_100M: " << port.child("AllowedSpeed_100M").child_value() << std::endl;
        std::cout << "  AllowedSpeed_1G: " << port.child("AllowedSpeed_1G").child_value() << std::endl;
        std::cout << "  AllowedSpeed_10G: " << port.child("AllowedSpeed_10G").child_value() << std::endl;
        std::cout << "  Default VLAN: " << default_vlan[id] << std::endl;
    }

    return 0;
}
