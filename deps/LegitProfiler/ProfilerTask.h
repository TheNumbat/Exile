namespace LegitProfiler
{
#define RGBA_LE(col) (((col & 0xff000000) >> (3 * 8)) + ((col & 0x00ff0000) >> (1 * 8)) + ((col & 0x0000ff00) << (1 * 8)) + ((col & 0x000000ff) << (3 * 8)))
  enum class Color : u32
  {
    turqoise = RGBA_LE(0x1abc9cffu),
    greenSea = RGBA_LE(0x16a085ffu),

    emerald = RGBA_LE(0x2ecc71ffu),
    nephritis = RGBA_LE(0x27ae60ffu),

    peterRiver = RGBA_LE(0x3498dbffu),
    belizeHole = RGBA_LE(0x2980b9ffu),

    amethyst = RGBA_LE(0x9b59b6ffu),
    wisteria = RGBA_LE(0x8e44adffu),

    sunFlower = RGBA_LE(0xf1c40fffu),
    orange = RGBA_LE(0xf39c12ffu),

    carrot = RGBA_LE(0xe67e22ffu),
    pumpkin = RGBA_LE(0xd35400ffu),

    alizarin = RGBA_LE(0xe74c3cffu),
    pomegranate = RGBA_LE(0xc0392bffu),

    clouds = RGBA_LE(0xecf0f1ffu),
    silver = RGBA_LE(0xbdc3c7ffu),
    imguiText = RGBA_LE(0xF2F5FAFFu)
  };
  struct Task
  {
    double startTime;
    double endTime;
    std::string name;
    Color color;
    double GetLength()
    {
      return endTime - startTime;
    }
  };
}