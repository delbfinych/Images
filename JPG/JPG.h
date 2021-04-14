#pragma once
#include "../Image.h"

class JPG : public Image {
    void save(const std::string& fileName) override;

public:
    JPG(const std::string& fileName);
};
