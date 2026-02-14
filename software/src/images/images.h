#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

#define Image1xHeight 7
#define Image1xWidth 5

typedef uint8_t Image1x[Image1xWidth];

#define ImageWidth2x 12
#define ImageWidth2xHalfSize ImageWidth2x;
#define ImageWidth2xFullSize (ImageWidth2x * 2)
#define ImageHeight2x 16

typedef uint8_t Image2x[ImageWidth2xFullSize];

enum Image2xId {
  Zero = 0,
  One,
  Two,
  Three,
  Four,
  Five,
  Six,
  Seven,
  Eight,
  Nine,
  Blank,
  Minus,
  IMG_2X_COUNT,
};

enum Image1xId {
  Fire0 = 0,
  Fire1,
  Fire2,
  Fire3,
  Fire4,
  Fire5,
  Fire6,
  Fire7,
  Fire8,
  Cold0,
  Cold1,
  Cold2,
  Cold3,
  Cold4,
  Cold5,
  Cold6,
  Cold7,
  Cold8,
  Idle0,
  IMG_1X_COUNT,
};

void LoadImage2x(Image2x* dest, enum Image2xId id);
uint8_t LoadImage2xByte(enum Image2xId, uint8_t offset);

void LoadImage1x(Image1x* dest, enum Image1xId id);
uint8_t LoadImage1xByte(enum Image1xId, uint8_t offset);

#if defined(__cplusplus)
}
#endif