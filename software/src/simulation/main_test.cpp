#include <lodepng.h>
#include <sys/types.h>

#include <iostream>

#include "temperature.hpp"

int main() {
  auto temp = Temperature::FromCelcius(Temperature::MinCelciusValue);

  temp.ChangeByMibiCelcius(1, false);
  std::cout << temp.GetCelciusWhole() << std::endl;

  temp.ChangeByMibiCelcius(10 * Temperature::MibiFactor, true);
  std::cout << temp.GetCelciusWhole() << std::endl;

  return 0;
}
