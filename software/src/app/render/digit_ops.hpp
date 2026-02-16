#pragma once

auto Hundreds(auto number) {
  return abs(number) / 100;
}

auto Tens(auto number) {
  return (abs(number) % 100) / 10;
}

auto Ones(auto number) {
  return abs(number) % 10;
}
