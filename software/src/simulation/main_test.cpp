#include <lodepng.h>
#include <sys/types.h>

#include <iostream>

#include "temperature.hpp"

int main() {
  auto temp = Temperature::FromCelcius(Temperature::MaxCelciusValue);

  std::cout << temp.GetFahrenheitWhole() << std::endl;

  std::cout << temp.GetCelciusWhole() << std::endl;

  return 0;
}
