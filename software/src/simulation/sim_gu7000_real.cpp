#include "sim_gu7000_real.hpp"

#include <algorithm>
#include <cstdint>
#include <istream>
#include <sstream>

namespace Font {
#include "font.h"
}

SimGu7000Real::SimGu7000Real()
    : display_memory_(),
      cursor_x_(0),
      cursor_y_(0),
      international_font_set_(0),
      character_code_type_(0),
      overwrite_mode_(false),
      scroll_mode_(0),
      horizontal_scroll_speed_(0),
      brightness_level_(8),
      reverse_display_(false),
      composition_mode_(0),
      current_window_(0),
      font_magnification_x_(1),
      font_magnification_y_(1) {}

void SimGu7000Real::ProcessCommand(const std::vector<uint8_t>& command) {
  if (command.empty()) {
    return;
  }

  std::basic_stringbuf<uint8_t> buf;
  std::basic_iostream<uint8_t> stream(&buf);
  stream.write(command.data(), command.size());

  uint8_t first_byte = stream.get();

  if (command.size() == 1) {  // Handle single-byte commands
    ProcessSingleByteCommand(first_byte);
  } else if (first_byte == 0x1B) {  // ESC commands
    ProcessEscCommand(stream);
  } else if (first_byte == 0x1F) {  // US commands
    ProcessUsCommand(stream);
  }
}

void SimGu7000Real::ProcessSingleByteCommand(uint8_t byte) {
  if (byte >= CMD_CHARACTER_DISPLAY_START &&
      byte <= CMD_CHARACTER_DISPLAY_END) {
    ProcessCharacterDisplay(byte);
  } else {
    switch (byte) {
      case CMD_BACKSPACE:
        ProcessBackspace();
        break;
      case CMD_HORIZONTAL_TAB:
        ProcessHorizontalTab();
        break;
      case CMD_LINE_FEED:
        ProcessLineFeed();
        break;
      case CMD_HOME_POSITION:
        ProcessHomePosition();
        break;
      case CMD_CARRIAGE_RETURN:
        ProcessCarriageReturn();
        break;
      case CMD_DISPLAY_CLEAR:
        ProcessDisplayClear();
        break;
      default:
        break;
    }
  }
}

void SimGu7000Real::ProcessEscCommand(Stream& command) {
  switch (command.get()) {
    case 0x40:  // Initialize Display
      ProcessInitializeDisplay();
      break;
    case 0x25:  // Specify Download Register
      ProcessSpecifyDownloadRegister(command);
      break;
    case 0x26:  // Download Character
      ProcessDownloadCharacter(command);
      break;
    case 0x3F:  // Delete Downloaded Character
      ProcessDeleteDownloadedCharacter(command);
      break;
    case 0x52:  // Specify International Font Set
      ProcessInternationalFontSet(command);
      break;
    case 0x74:  // Specify Character Code Type
      ProcessCharacterCodeType(command);
      break;
    default:
      break;
  }
}
void SimGu7000Real::ProcessUsCommand(Stream& command) {
  switch (command.get()) {
    case 0x01:  // Overwrite Mode
      ProcessOverwriteMode(command);
      break;
    case 0x02:  // Vertical Scroll Mode
      ProcessVerticalScrollMode(command);
      break;
    case 0x03:  // Horizontal Scroll Mode
      ProcessHorizontalScrollMode(command);
      break;
    case 0x24:  // Cursor Set
      ProcessCursorSet(command);
      break;
    case 0x28:  // Extended commands
      ProcessUsExtendedCommands(command);
      break;
    case 0x58:  // Brightness Control
      ProcessBrightnessControl(command);
      break;
    case 0x72:  // Specify or Cancel Reverse Display
      ProcessReverseDisplay(command);
      break;
    case 0x73:  // Horizontal Scroll Speed
      ProcessHorizontalScrollSpeed(command);
      break;
    case 0x77:  // Specify Write Mixture Display Mode
      ProcessCompositionMode(command);
      break;
    default:
      break;
  }
}

void SimGu7000Real::ProcessUsExtendedCommands(Stream& command) {
  switch (command.get()) {
    case 0x61:  // Wait / Scroll Display Action / Display Blink /
      switch (command.get()) {
        case 0x01:  // Wait
          ProcessWait(command);
          break;
        case 0x10:  // Scroll Display Action
          ProcessScrollDisplayAction(command);
          break;
        case 0x11:  // Display Blink
          ProcessDisplayBlink(command);
          break;
        case 0x40:  // Screen Saver
          ProcessScreenSaver(command);
          break;
        default:
          break;
      }
      break;
    case 0x64:
      switch (command.get()) {
        case 0x21:  // Real Time Bit Image Display
          ProcessRealTimeBitImageDisplayXy(command);
          break;
        default:
          break;
      }
      break;
    case 0x66:  // Image Display
      switch (command.get()) {
        case 0x11:  // Real Time Bit Image Display
          ProcessRealTimeBitImageDisplay(command, cursor_x_, cursor_y_);
          break;
        default:
          break;
      }
      break;
    case 0x67:  // Character Font Width and Space
      switch (command.get()) {
        case 0x03:  // Character Font Width and Space
          ProcessCharacterFontWidthAndSpace(command);
          break;
        case 0x40:  // Font Magnification Set
          ProcessFontMagnificationSet(command);
          break;
        default:
          break;
      }
      break;
    case 0x77:  // Window commands
      switch (command.get()) {
        case 0x01:  // Current Window Select
          ProcessCurrentWindowSelect(command);
          break;
        case 0x02:  // User Window definition-cancel
          ProcessUserWindowDefinitionCancel(command);
          break;
        case 0x10:  // Write Screen Mode Select
          ProcessWriteScreenModeSelect(command);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

const SimGu7000Real::DisplayMemory& SimGu7000Real::GetDisplayMemory() const {
  return display_memory_;
}

void SimGu7000Real::ClearDisplayMemory() {
  for (auto& column : display_memory_) {
    column.fill(false);
  }
}

void SimGu7000Real::SetCursor(uint16_t x, uint16_t y) {
  cursor_x_ = std::min(x, static_cast<uint16_t>(DISPLAY_WIDTH - 1));
  cursor_y_ = std::min(y, static_cast<uint16_t>(DISPLAY_HEIGHT - 1));
}

uint16_t SimGu7000Real::GetCursorX() const {
  return cursor_x_;
}

uint16_t SimGu7000Real::GetCursorY() const {
  return cursor_y_;
}

void SimGu7000Real::DrawCharacter(uint8_t character) {
  DrawFontCharacter(cursor_x_, cursor_y_, character);

  // Advance cursor
  cursor_x_ += FONT_WIDTH * font_magnification_x_;
  if (cursor_x_ >= DISPLAY_WIDTH) {
    cursor_x_ = 0;
    cursor_y_ += FONT_HEIGHT * font_magnification_y_;
    if (cursor_y_ >= DISPLAY_HEIGHT) {
      cursor_y_ = 0;
    }
  }
}

void SimGu7000Real::DrawCharacterAt(uint16_t x, uint16_t y, uint8_t character) {
  DrawFontCharacter(x, y, character);
}

// Helper methods
void SimGu7000Real::SetPixel(uint16_t x, uint16_t y, bool on) {
  if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
    return;
  }

  display_memory_[x][y] = on;
}

bool SimGu7000Real::GetPixel(uint16_t x, uint16_t y) const {
  if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
    return false;
  }

  return display_memory_[x][y];
}

void SimGu7000Real::DrawFontCharacter(uint16_t x, uint16_t y,
                                      uint8_t character) {
  auto font_data = GetFontData(character);
  DrawImage(x, y, font_data, FONT_WIDTH, FONT_HEIGHT);
}

void SimGu7000Real::DrawImage(uint16_t x, uint16_t y,
                              const FontCharacter& image, uint8_t width,
                              uint8_t height) {
  if (image.empty()) {
    return;
  }

  // Draw each column of the character
  for (uint8_t col = 0; col < width; ++col) {
    uint8_t column_data = image[col];

    // Draw each pixel in the column with magnification
    for (uint8_t row = 0; row < height; ++row) {
      bool pixel_on = (column_data & (1 << row)) != 0;

      // Apply magnification
      for (uint8_t mx = 0; mx < font_magnification_x_; ++mx) {
        for (uint8_t my = 0; my < font_magnification_y_; ++my) {
          uint16_t pixel_x = x + col * font_magnification_x_ + mx;
          uint16_t pixel_y = y + row * font_magnification_y_ + my;

          if (pixel_x < DISPLAY_WIDTH && pixel_y < DISPLAY_HEIGHT) {
            SetPixel(pixel_x, pixel_y, pixel_on);
          }
        }
      }
    }
  }
}

SimGu7000Real::FontCharacter SimGu7000Real::GetFontData(
    uint8_t character) const {
  auto GetTheData = [](char c) {
    auto begin = &Font::font.Bitmap[(c - 31) * Font::font.Height];
    return FontCharacter(begin, Font::font.Height);
  };

  if (character < 32 || character > 126) {
    return GetTheData(' ');  // Return space for invalid characters
  }

  return GetTheData(character);
}

// Command implementations
void SimGu7000Real::ProcessCharacterDisplay(uint8_t character) {
  DrawCharacter(character);
}

void SimGu7000Real::ProcessBackspace() {
  if (cursor_x_ >= FONT_WIDTH * font_magnification_x_) {
    cursor_x_ -= FONT_WIDTH * font_magnification_x_;
  } else {
    cursor_x_ = DISPLAY_WIDTH - FONT_WIDTH * font_magnification_x_;
    if (cursor_y_ >= FONT_HEIGHT * font_magnification_y_) {
      cursor_y_ -= FONT_HEIGHT * font_magnification_y_;
    } else {
      cursor_y_ = DISPLAY_HEIGHT - FONT_HEIGHT * font_magnification_y_;
    }
  }
}

void SimGu7000Real::ProcessHorizontalTab() {
  uint16_t tab_width =
      FONT_WIDTH * font_magnification_x_ * 4;  // Tab = 4 characters
  cursor_x_ = ((cursor_x_ / tab_width) + 1) * tab_width;
  if (cursor_x_ >= DISPLAY_WIDTH) {
    cursor_x_ = 0;
    cursor_y_ += FONT_HEIGHT * font_magnification_y_;
    if (cursor_y_ >= DISPLAY_HEIGHT) {
      cursor_y_ = 0;
    }
  }
}

void SimGu7000Real::ProcessLineFeed() {
  cursor_y_ += FONT_HEIGHT * font_magnification_y_;
  if (cursor_y_ >= DISPLAY_HEIGHT) {
    cursor_y_ = 0;
  }
}

void SimGu7000Real::ProcessHomePosition() {
  cursor_x_ = 0;
  cursor_y_ = 0;
}

void SimGu7000Real::ProcessCarriageReturn() {
  cursor_x_ = 0;
}

void SimGu7000Real::ProcessDisplayClear() {
  ClearDisplayMemory();
  ProcessHomePosition();
}

void SimGu7000Real::ProcessInitializeDisplay() {
  ClearDisplayMemory();
  cursor_x_ = 0;
  cursor_y_ = 0;
  international_font_set_ = 0;
  character_code_type_ = 0;
  overwrite_mode_ = false;
  scroll_mode_ = 0;
  horizontal_scroll_speed_ = 0;
  brightness_level_ = 8;
  reverse_display_ = false;
  composition_mode_ = 0;
  current_window_ = 0;
  font_magnification_x_ = 1;
  font_magnification_y_ = 1;
}

void SimGu7000Real::ProcessCursorSet(Stream& params) {
  uint8_t x, y;
  ExtractXY(params, x, y);
  SetCursor(x, y);
}

void SimGu7000Real::ProcessDisplayClearCommand() {
  ProcessDisplayClear();
}

void SimGu7000Real::ProcessBrightnessControl(Stream& params) {
  brightness_level_ = std::clamp(params.get(), 1ul, 8ul);
}

void SimGu7000Real::ProcessReverseDisplay(Stream& params) {
  reverse_display_ = (params.get() != 0);
}

void SimGu7000Real::ProcessHorizontalScrollSpeed(Stream& params) {
  horizontal_scroll_speed_ = std::clamp(params.get(), 0ul, 31ul);
}

void SimGu7000Real::ProcessCompositionMode(Stream& params) {
  composition_mode_ = std::clamp(params.get(), 0ul, 3ul);
}

void SimGu7000Real::ProcessInternationalFontSet(Stream& params) {
  international_font_set_ = params.get();
}

void SimGu7000Real::ProcessCharacterCodeType(Stream& params) {
  character_code_type_ = params.get();
}

void SimGu7000Real::ProcessOverwriteMode(Stream& params) {
  overwrite_mode_ = (params.get() == 0x01);
}

void SimGu7000Real::ProcessVerticalScrollMode(Stream& params) {
  scroll_mode_ = 2;  // Vertical scroll
}

void SimGu7000Real::ProcessHorizontalScrollMode(Stream& params) {
  scroll_mode_ = 3;  // Horizontal scroll
}

void SimGu7000Real::ProcessWait(Stream& params) {
  // Wait command - in simulation, we just ignore timing
  // In real hardware this would wait for t*0.5 seconds
}

void SimGu7000Real::ProcessScrollDisplayAction(Stream& params) {
  // Scroll display action - complex scrolling implementation
  // For now, just stub this out
}

void SimGu7000Real::ProcessDisplayBlink(Stream& params) {
  // Display blink - complex blinking implementation
  // For now, just stub this out
}

void SimGu7000Real::ProcessScreenSaver(Stream& params) {
  switch (params.get()) {
    case 0:  // Power save
    case 2:  // All dots off
      ClearDisplayMemory();
      break;
    case 3:  // All dots on
      for (auto& column : display_memory_) {
        column.fill(true);
      }
      break;
    case 1:  // Power on
    case 4:  // Repeat normal & reverse display
    default:
      // No change for other modes
      break;
  }
}

void SimGu7000Real::ProcessRealTimeBitImageDisplayXy(Stream& params) {
  uint8_t x, y;
  ExtractXY(params, x, y);
  ProcessRealTimeBitImageDisplay(params, x, y);
}

void SimGu7000Real::ProcessRealTimeBitImageDisplay(Stream& params, uint8_t x,
                                                   uint8_t y) {
  uint8_t w, h, g;
  ExtractXY(params, w, h);
  params >> g;
  for (uint8_t i = 0; i < (h / 8) * w; i++) {
    uint8_t byte = params.get();
    for (uint8_t j = 0; j < h; ++j) {
      display_memory_[x + i][y + j] = byte & (1 << j);
    }
  }
}

void SimGu7000Real::ProcessCharacterFontWidthAndSpace(Stream& params) {
  // Character font width and space - font spacing implementation
  // For now, just stub this out
}

void SimGu7000Real::ProcessFontMagnificationSet(Stream& params) {
  font_magnification_x_ = std::clamp(params.get(), 1ul, 4ul);
  font_magnification_y_ = std::clamp(params.get(), 1ul, 2ul);
}

void SimGu7000Real::ProcessCurrentWindowSelect(Stream& params) {
  current_window_ = std::clamp(params.get(), 0ul, 4ul);
}

void SimGu7000Real::ProcessUserWindowDefinitionCancel(Stream& params) {
  // User window definition/cancel - complex window management
  // For now, just stub this out
}

void SimGu7000Real::ProcessWriteScreenModeSelect(Stream& params) {
  // Write screen mode select - screen mode management
  // For now, just stub this out
}

void SimGu7000Real::ProcessSpecifyDownloadRegister(Stream& params) {
  // Specify download register - download enable/disable
  // For now, just stub this out
}

void SimGu7000Real::ProcessDownloadCharacter(Stream& params) {
  // Download character - custom character definition
  // For now, just stub this out
}

void SimGu7000Real::ProcessDeleteDownloadedCharacter(Stream& params) {
  // Delete downloaded character - custom character removal
  // For now, just stub this out
}

void SimGu7000Real::ExtractXY(Stream& s, uint8_t& x, uint8_t& y) {
  uint8_t xl, xh, yl, yh;
  s >> xl >> xh >> yl >> yh;
  x = (xh << 8) | xl;
  y = (yh << 8) | yl;
}