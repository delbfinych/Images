#pragma once

#include <string>
#include "../Image.h"

class PNG : public Image {
    void save(const std::string& fileName) override;

public:
    PNG(const std::string& fileName);
};