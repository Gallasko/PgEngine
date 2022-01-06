#pragma once

#include <string>
#include <memory>

class Serializer
{
    Serializer(const std::string& filename) : filename(filename) {}

public:
    static std::unique_ptr<Serializer>& getSerializer(const std::string& filename) {static std::unique_ptr<Serializer> serializer = std::make_unique<Serializer>(filename); return serializer; }

    

private:
    std::string filename;

};