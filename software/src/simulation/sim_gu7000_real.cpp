#include "sim_gu7000_real.hpp"

#include <algorithm>
#include <cstdint>

namespace Font {
#include "font.h"
}

SimGu7000Real::SimGu7000Real() {}

void SimGu7000Real::ProcessCommand(uint8_t byte) {
  if (byte >= CMD_CHARACTER_DISPLAY_START &&
      byte <= CMD_CHARACTER_DISPLAY_END) {
    ProcessCharacterDisplay(byte);
    return;
  }

  if (state_ == State::LookingForCommand) {
    command_buffer_.push_back(byte);

    auto command = std::find_if(
        CommandTable.begin(), CommandTable.end(),
        [&](const auto& x) { return x.Prefix == command_buffer_; });

    if (command != CommandTable.end()) {
      CurrentCommand_ = command;
      if (command->FixedArgumentBytes == 0) {
        (this->*CurrentCommand_->Execute)(command_arguments_);
        state_ = State::LookingForCommand;
      } else {
        state_ = State::GettingCommandArguments;
      }
    }
  } else if (state_ == State::GettingCommandArguments) {
    command_arguments_.push_back(byte);
    if (command_arguments_.size() == CurrentCommand_->FixedArgumentBytes) {
      if (CurrentCommand_->SizeGetFn) {
        CurrentCommandVariableBytes_ =
            (this->*CurrentCommand_->SizeGetFn)(command_arguments_);
        if (CurrentCommandVariableBytes_ > 0) {
          state_ = State::GettingVariableArgs;
        } else {
          (this->*CurrentCommand_->Execute)(command_arguments_);
          state_ = State::LookingForCommand;
        }
      } else {
        (this->*CurrentCommand_->Execute)(command_arguments_);
        state_ = State::LookingForCommand;
      }
    }
  } else if (state_ == State::GettingVariableArgs) {
    command_arguments_.push_back(byte);
    if (command_arguments_.size() ==
        (CurrentCommand_->FixedArgumentBytes + CurrentCommandVariableBytes_)) {
      (this->*CurrentCommand_->Execute)(command_arguments_);
      state_ = State::LookingForCommand;
    }
  }
}

SimGu7000Real::CommandTableA SimGu7000Real::CommandTable = {{
    // Single byte commands
    {
        .Prefix = "\x08",
        .Execute = &SimGu7000Real::ProcessBackspace,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    {
        .Prefix = "\x09",
        .Execute = &SimGu7000Real::ProcessHorizontalTab,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    {
        .Prefix = "\x0A",
        .Execute = &SimGu7000Real::ProcessLineFeed,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    {
        .Prefix = "\x0B",
        .Execute = &SimGu7000Real::ProcessHomePosition,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    {
        .Prefix = "\x0C",
        .Execute = &SimGu7000Real::ProcessDisplayClearCommand,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    {
        .Prefix = "\x0D",
        .Execute = &SimGu7000Real::ProcessCarriageReturn,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    // ESC commands (\x1B prefix)
    {
        .Prefix = "\x1B\x40",
        .Execute = &SimGu7000Real::ProcessInitializeDisplay,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    {
        .Prefix = "\x1B\x25",
        .Execute = &SimGu7000Real::ProcessSpecifyDownloadRegister,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    {
        .Prefix = "\x1B\x26",
        .Execute = &SimGu7000Real::ProcessDownloadCharacter,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    {
        .Prefix = "\x1B\x3F",
        .Execute = &SimGu7000Real::ProcessDeleteDownloadedCharacter,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    {
        .Prefix = "\x1B\x52",
        .Execute = &SimGu7000Real::ProcessInternationalFontSet,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    {
        .Prefix = "\x1B\x74",
        .Execute = &SimGu7000Real::ProcessCharacterCodeType,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    // US commands (\x1F\x28 prefix)
    {
        .Prefix = "\x1F\x28\x01",
        .Execute = &SimGu7000Real::ProcessOverwriteMode,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 1,
    },
    {
        .Prefix = "\x1F\x28\x02",
        .Execute = &SimGu7000Real::ProcessVerticalScrollMode,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    {
        .Prefix = "\x1F\x28\x03",
        .Execute = &SimGu7000Real::ProcessHorizontalScrollMode,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    {
        .Prefix = "\x1F\x28\x24",
        .Execute = &SimGu7000Real::ProcessCursorSet,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes =
            4,  // 2 bytes for x (low, high), 2 bytes for y (low, high)
    },
    {
        .Prefix = "\x1F\x28\x58",
        .Execute = &SimGu7000Real::ProcessBrightnessControl,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 1,
    },
    {
        .Prefix = "\x1F\x28\x72",
        .Execute = &SimGu7000Real::ProcessReverseDisplay,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 1,
    },
    {
        .Prefix = "\x1F\x28\x73",
        .Execute = &SimGu7000Real::ProcessHorizontalScrollSpeed,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 1,
    },
    {
        .Prefix = "\x1F\x28\x77",
        .Execute = &SimGu7000Real::ProcessCompositionMode,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 1,
    },
    // US Extended commands (\x1F\x28 prefix with sub-commands)
    {
        .Prefix = "\x1F\x28\x61\x01",
        .Execute = &SimGu7000Real::ProcessWait,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    {
        .Prefix = "\x1F\x28\x61\x10",
        .Execute = &SimGu7000Real::ProcessScrollDisplayAction,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    {
        .Prefix = "\x1F\x28\x61\x11",
        .Execute = &SimGu7000Real::ProcessDisplayBlink,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    {
        .Prefix = "\x1F\x28\x61\x40",
        .Execute = &SimGu7000Real::ProcessScreenSaver,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 1,
    },
    {
        .Prefix = "\x1F\x28\x64\x21",
        .Execute = &SimGu7000Real::ProcessRealTimeBitImageDisplayXy,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 9,  // 2 bytes x, 2 bytes y, 1 byte width, 1
                                  // byte height, 1 byte g, 2 bytes data
    },
    // Note: ProcessRealTimeBitImageDisplay has a different signature (3
    // params)
    // and is not in CommandTable. It's called internally by
    // ProcessRealTimeBitImageDisplayXy
    {
        .Prefix = "\x1F\x28\x67\x03",
        .Execute = &SimGu7000Real::ProcessCharacterFontWidthAndSpace,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 1,
    },
    {
        .Prefix = "\x1F\x28\x67\x40",
        .Execute = &SimGu7000Real::ProcessFontMagnificationSet,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 2,
    },
    {
        .Prefix = "\x1F\x28\x77\x01",
        .Execute = &SimGu7000Real::ProcessCurrentWindowSelect,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 1,
    },
    {
        .Prefix = "\x1F\x28\x77\x02",
        .Execute = &SimGu7000Real::ProcessUserWindowDefinitionCancel,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 0,
    },
    {
        .Prefix = "\x1F\x28\x77\x10",
        .Execute = &SimGu7000Real::ProcessWriteScreenModeSelect,
        .SizeGetFn = nullptr,
        .FixedArgumentBytes = 1,
    },
}};

// Size getter functions for variable-length commands
// These would be called to determine how many additional bytes follow the fixed
// arguments For now, they return 0 since most commands don't have variable
// arguments

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

void SimGu7000Real::DrawImage(uint16_t x, uint16_t y, const FontCharSpan& image,
                              uint8_t width, uint8_t height) {
  if (image.empty()) {
    return;
  }

  // Draw each column of the character
  for (uint8_t row = 0; row < height; ++row) {
    int row_data = image[row];

    // Draw each pixel in the column with magnification
    for (uint8_t col = 0; col < width; ++col) {
      bool pixel_on = (row_data & (1 << (7 - col))) != 0;

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

SimGu7000Real::FontCharSpan SimGu7000Real::GetFontData(
    uint8_t character) const {
  auto GetTheData = [](char c) {
    auto begin = &Font::font.Bitmap[(c - 31) * Font::font.Height];
    return FontCharSpan(begin, Font::font.Height);
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

void SimGu7000Real::ProcessBackspace(Stream&) {
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

void SimGu7000Real::ProcessHorizontalTab(Stream&) {
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

void SimGu7000Real::ProcessLineFeed(Stream&) {
  cursor_y_ += FONT_HEIGHT * font_magnification_y_;
  if (cursor_y_ >= DISPLAY_HEIGHT) {
    cursor_y_ = 0;
  }
}

void SimGu7000Real::ProcessHomePosition(Stream&) {
  cursor_x_ = 0;
  cursor_y_ = 0;
}

void SimGu7000Real::ProcessCarriageReturn(Stream&) {
  cursor_x_ = 0;
}

void SimGu7000Real::ProcessDisplayClear(Stream& params) {
  ClearDisplayMemory();
  ProcessHomePosition(params);
}

void SimGu7000Real::ProcessInitializeDisplay(Stream&) {
  ClearDisplayMemory();
  state_ = State::LookingForCommand;
  initialized_ = true;
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

void SimGu7000Real::ProcessDisplayClearCommand(Stream& params) {
  ProcessDisplayClear(params);
}

void SimGu7000Real::ProcessBrightnessControl(Stream& params) {
  if (!params.empty()) {
    brightness_level_ = std::min(uint8_t(8), std::max(uint8_t(1), params[0]));
  }
}

void SimGu7000Real::ProcessReverseDisplay(Stream& params) {
  if (!params.empty()) {
    reverse_display_ = (params[0] != 0);
  }
}

void SimGu7000Real::ProcessHorizontalScrollSpeed(Stream& params) {
  if (!params.empty()) {
    horizontal_scroll_speed_ =
        std::min(uint8_t(31), std::max(uint8_t(0), params[0]));
  }
}

void SimGu7000Real::ProcessCompositionMode(Stream& params) {
  if (!params.empty()) {
    composition_mode_ = std::min(uint8_t(3), std::max(uint8_t(0), params[0]));
  }
}

void SimGu7000Real::ProcessInternationalFontSet(Stream& params) {
  if (!params.empty()) {
    international_font_set_ = params[0];
  }
}

void SimGu7000Real::ProcessCharacterCodeType(Stream& params) {
  if (!params.empty()) {
    character_code_type_ = params[0];
  }
}

void SimGu7000Real::ProcessOverwriteMode(Stream& params) {
  if (!params.empty()) {
    overwrite_mode_ = (params[0] == 0x01);
  }
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
  if (params.empty()) return;
  switch (params[0]) {
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
  if (params.size() < 5) return;  // Need at least width, height, and g
  uint8_t w = params[0];
  uint8_t h = params[1];
  // g value at params[4] is not used in simulation
  size_t data_offset = 5;

  for (uint8_t i = 0; i < (h / 8) * w && data_offset < params.size();
       i++, data_offset++) {
    uint8_t byte = params[data_offset];
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
  if (params.size() >= 2) {
    font_magnification_x_ =
        std::min(uint8_t(4), std::max(uint8_t(1), params[0]));
    font_magnification_y_ =
        std::min(uint8_t(2), std::max(uint8_t(1), params[1]));
  }
}

void SimGu7000Real::ProcessCurrentWindowSelect(Stream& params) {
  if (!params.empty()) {
    current_window_ = std::min(uint8_t(4), std::max(uint8_t(0), params[0]));
  }
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
  if (s.size() >= 4) {
    uint8_t xl = s[0], xh = s[1], yl = s[2], yh = s[3];
    x = (xh << 8) | xl;
    y = (yh << 8) | yl;
  }
}