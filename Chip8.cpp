
#include "Chip8.h"
#include <iomanip>
#include <random>

#define DEBUG false

Chip8::Chip8()
    : displayModule(m_sdlEventHandler),
      inputModule(m_sdlEventHandler),
      memory(4096, 0)
{
  init();
  srand(0);
}

void Chip8::init()
{
  // Initialise registers and memory once;

  pc = 0x200; // Program counter starts at this address;
  opcode = 0;
  I = 0;
  sp = 0;

  // Clear Display
  displayModule.clearScreen();

  // Clear registers V0-VF and stack
  memset(&V, 0, 0xF);
  memset(&stack, 0, 0xF);

  // Load fontset
  auto fontset = displayModule.getFontset();
  for (int i = 0; i < 80; ++i)
  {
    std::cout << std::hex << std::setw(4) << std::setfill('0')
              << "Fontset[" << i << "] = "
              << static_cast<int>(fontset[i]) << "\n";
    memory[i] = fontset[i];
  }
}

void Chip8::handleEvents()
{
  m_sdlEventHandler.handleEvents();
}

void Chip8::updateDisplay()
{
  displayModule.updateScreen();
}

bool Chip8::load(const std::string &file)
{

  std::ifstream reader(file, std::ios::binary);

  if (reader.good())
  {

    int i = 0;
    char c;
    std::cout << "\n0x" << std::setw(4) << std::setfill('0')
              << static_cast<int>(0) << " ";

    // (while (reader >> c)) skips 0x20, space character!
    while (!reader.eof())
    {

      reader.read(&c, sizeof(char));

      std::cout << std::hex << std::setfill('0')
                << std::setw(2) << static_cast<int>(c & 0xFF) << " ";

      memory[i + pc] = c & 0xFF;
      i++;

      if (i % 0x10 == 0)
      {
        std::cout << "\n0x" << std::setw(4) << std::setfill('0')
                  << static_cast<int>(i) << " ";
      }
    }

    std::cout << "i: " << i << "\n";
  }
  else
  {

    std::cout << "Failed to open " << file << "\n";
    return false;
  }

  return true;
}

void Chip8::emulateCycle()
{

  // An opcode is 2 bytes long; need to fetch 2 successive bytes of the program counter
  opcode = memory[pc] << 8 | memory[pc + 1]; // Join into 2 bytes

  if (DEBUG)
  {
    std::cout << std::setw(2) << std::setfill('0') << std::hex
              << "memory[" << pc << "] << 8 = "
              << static_cast<int>(memory[pc] << 8) << "\n"
              << "memory[" << pc << " + 1] << 8 = "
              << static_cast<int>(memory[pc + 1]) << "\n";

    std::cout << "opcode: " << std::setw(4) << std::setfill('0') << std::hex << opcode << "\n";
    std::cout << "pc: " << std::dec << pc << "\n";
  }

  uint8_t x = (opcode >> 8) & 0x000F; // the lower 4 bits of the high byte
  uint8_t y = (opcode >> 4) & 0x000F; // the upper 4 bits of the low byte
  uint8_t n = opcode & 0x000F;        // the lowest 4 bits
  uint8_t kk = opcode & 0x00FF;       // the lowest 8 bits
  uint16_t nnn = opcode & 0x0FFF;     // the lowest 12 bits

  auto beforeOpPc = pc;

  switch (opcode & 0xF000)
  {
  case 0x0000:
  {
    switch (kk)
    {
    case 0x00E0:
    {
      // Clears screen
      if (DEBUG)
      {
        std::cout << "Clearing screen\n";
      }
      displayModule.clearScreen();
      pc += 2;
      break;
    }

    case 0x00EE:
    {
      // 0x00EE: Returns from subroutine

      if (DEBUG)
      {
        std::cout << "Stack at level " << sp << " = " << pc << "\n";
        std::cout << "stack[sp] = " << stack[sp] << "\n";
      }

      pc = stack[--sp];

      if (DEBUG)
      {
        std::cout << "pc = " << pc << "\n";
      }
      break;
    }
    default:
    {
      std::cout << "Invalid opcode: " << std::setw(4)
                << std::setfill('0') << opcode << "\n";
      break;
    }
    }
    break;
  }

  case 0x1000:
  {
    pc = nnn;
    break;
  }

  case 0x2000:
  {
    stack[sp] = pc + 2;
    ++sp;
    if (DEBUG)
    {
      std::cout << "Stack at level " << sp << " = " << pc << "\n";
    }
    pc = nnn;
    break;
  }

  case 0x3000:
  {
    V[x] == kk ? pc += 4 : pc += 2;
    break;
  }

  case 0x4000:
  {
    V[x] != kk ? pc += 4 : pc += 2;
    break;
  }

  case 0x5000:
  {
    V[x] == V[y] ? pc += 4 : pc += 2;
    break;
  }

  case 0x6000:
  {
    V[x] = kk;
    pc += 2;
    break;
  }

  case 0x7000:
  {
    V[x] += kk;
    pc += 2;
    break;
  }

  case 0x8000:
  {
    switch (opcode & 0x000F)
    {

    case 0x0000:
    {
      V[x] = V[y];
      break;
    }

    case 0x0001:
    {
      V[x] |= V[y];
      break;
    }

    case 0x0002:
    {
      V[x] &= V[y];
      break;
    }

    case 0x0003:
    {
      V[x] ^= V[y];
      break;
    }

    case 0x0004:
    {
      V[0xF] = (V[x] + V[y]) > 0xFF ? 1 : 0;
      V[x] += V[y];
      break;
    }

    case 0x0005:
    {
      V[0xF] = V[x] > V[y] ? 1 : 0;
      V[x] -= V[y];
      break;
    }

    case 0x0006:
    {
      V[0xF] = V[x] & 0x01;
      V[x] >>= 1; // divide by 2
      break;
    }

    case 0x0007:
    {
      V[0xF] = V[y] > V[x] ? 1 : 0;
      V[x] = V[y] - V[x];
      break;
    }

    case 0x000E:
    {
      V[0xF] = (V[x] >> 7) & 0x1;
      V[x] <<= 1; // multiply by 2
      break;
    }

    default:
    {
      std::cout << "Invalid opcode: " << std::setw(4)
                << std::setfill('0') << std::hex << opcode << "\n";
      break;
    }
    }
    pc += 2;
    break;
  }

  case 0x9000:
  {
    V[x] != V[y] ? pc += 4 : pc += 2;
    break;
  }

  case 0xA000:
  {
    I = nnn;
    pc += 2;
    break;
  }

  case 0xB000:
  {
    pc = nnn + V[0x0];
    break;
  }

  case 0xC000:
  {
    V[x] = (rand() % (0xFF + 1)) & kk;
    pc += 2;
    break;
  }

  case 0xD000:
  {
    auto col = V[x];
    auto row = V[y];
    auto height = n;
    uint8_t pixel;

    // Collision flag
    V[0xF] = 0;

    // Loop over each row
    for (int byteIndex = 0; byteIndex < height; ++byteIndex)
    {

      pixel = memory[I + byteIndex]; // Pixel value of memory starting at I

      if (DEBUG)
      {
        std::cout << "Memory location: "
                  << std::hex << std::setw(4) << std::setfill('0')
                  << I + byteIndex << "\n";
        std::cout << "Pixel: " << pixel << "\n";
      }

      // Loop over each column (8 bits of 1 row)
      for (int bitIndex = 0; bitIndex < 8; ++bitIndex)
      {

        // Check which pixels are 0, within a byte
        // by checking each one individually by shifting
        auto bit = (pixel >> bitIndex) & 0x1;

        auto colIndex = (col + bitIndex - 7) % DisplayModule::WIDTH;
        auto rowIndex = (row + byteIndex) % DisplayModule::HEIGHT;
        auto &screenPixel = displayModule.getGfxArray()[colIndex + rowIndex * DisplayModule::WIDTH];
        screenPixel ^= bit;

        // If the pixel is 1 and the pixel on the display is 1 then a collision is detected
        if (bit == 1 && screenPixel == 1)
        {
          V[0xF] = 1;
        }

        if (DEBUG)
        {
          std::cout << "Painting at col: " << std::dec
                    << colIndex << " row: " << rowIndex << "\n";
          std::cout << "ScreenPixel " << screenPixel << "\n";
        }
      }
    }
    displayModule.setDrawFlag();
    pc += 2;
    break;
  }

  case 0xE000:
  {
    switch (opcode & 0x00FF)
    {

    case 0x009E:
    {
      inputModule.isKeyPressed(V[x]) ? pc += 4 : pc += 2;
      break;
    }

    case 0x00A1:
    {
      !inputModule.isKeyPressed(V[x]) ? pc += 4 : pc += 2;
      break;
    }
    default:
    {
      std::cout << "Invalid opcode: " << std::setw(4)
                << std::setfill('0') << std::hex << opcode << "\n";
      break;
    }
    }
    break;
  }

  case 0xF000:
  {
    switch (opcode & 0x00FF)
    {

    case 0x0007:
    {
      V[x] = delayTimer;
      pc += 2;
    }

    case 0x000A:
    {
      V[x] = inputModule.waitUntilKeyPress();
      pc += 2;
    }

    case 0x0015:
    {
      delayTimer = V[x];
      pc += 2;
    }

    case 0x0018:
    {
      soundTimer = V[x];
      pc += 2;
      break;
    }

    case 0x001E:
    {
      V[0xF] = (I + V[x] > 0xFFF) ? 1 : 0;
      I += V[x];
      pc += 2;
      break;
    }

    case 0x0029:
    {
      I = V[x] * 0x05;
      pc += 2;
      break;
    }

    case 0x0033:
    {
      memory[I] = (V[x] % 1000) / 100;   // hundreds digit
      memory[I + 1] = (V[x] % 100) / 10; // tenth digit
      memory[I + 2] = (V[x] % 10);       // ones digit
      pc += 2;
      break;
    }

    case 0x0055:
    {
      std::cout << "HIT55\n";
      for (int i = 0; i <= x; ++i)
      {
        memory[I + i] = V[i];
      }
      I += x + 1;
      pc += 2;
      break;
    }

    case 0x0065:
    {
      std::cout << "HIT65\n";
      for (int i = 0; i <= x; ++i)
      {
        V[i] = memory[I + i];
      }
      I += x + 1;
      pc += 2;
      break;
    }

    default:
    {
      std::cout << "Invalid opcode: " << std::setw(4)
                << std::setfill('0') << std::hex << opcode << "\n";
      break;
    }
    }
  }
  break;
  }

  // Update timers
  if (delayTimer > 0)
  {
    --delayTimer;
  }

  if (soundTimer > 0)
  {
    --soundTimer;
    if (soundTimer == 1)
    {
      std::cout << "Beep!\n";
    }
  }
}
