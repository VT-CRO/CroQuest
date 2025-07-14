# 🕹️ CroQuest

**CroQuest** is a retro-style handheld game console powered by the ESP32 microcontroller. If features a custom UI, SD-card-loaded assets, Bluetooth multiplayer, and a variety of original games - all packed into a portable device.

---

## 📸 Preview

![CroQuest Cover](./images/cover.jpg)

---

- 🎮 8 preloaded games: Pong, Snake, Chess, Tic Tac Toe, Simon, Connect 4, Memory, Tetris, Breakout
- 🧠 Single & multiplayer modes via BLE
- 🔊 Audio control and custom sound effects
- 🏆 Unlockable badges and achievements
- 💾 SD card support for game assets and save data
- 📟 Animated menus, numeric input, and game state sync
- 🎨 Fully customizable user name, screen brightness, and profile badges

---

## 📁 Repository Structure

```bash
CroQuest
├── ESP32/                 # Main firmware and source code
│   ├── src/              # All game and system modules
│   ├── include/          # Header files
│   ├── lib/              # External libraries
│   └── Files/            # Educational diagrams and example scripts (PCB and 3D design files not included)
└── SD/                   # SD card content
    ├── assets/           # JPEG assets for games and UI
    └── saves/            # Saved data (e.g., badges, settings)
```

## ⚠️ License & Usage Terms

This repository is made public for educational and non-commercial use under the MIT License.

- You may use, modify, and share this firmware for learning, teaching, and non-profit projects.
- **Commercial use, resale, or distribution of products derived from this repository is strictly prohibited without explicit written permission.**
- The console’s commercial features — including block-based coding tools, licensing systems, and customer management platforms — are developed separately under a private repository and are not covered under this license.

For licensing inquiries or partnerships, please contact:  
Felipe Campoverde  
fcampoverdeg@vt.edu

## Future Development

CroQuest is actively evolving.

Planned features for future releases include:

- Private block-based coding platform for custom game development
- Dedicated educational kits for schools and training programs
- Expanded multiplayer features

These features are part of CroQuest's commercial version and are not included in this public repository.

## Important Notice

CroQuest was originally developed as part of VT CRO (Competitive Robotics Organization at Virginia Tech).

Ownership rights to all content in this repository are held by Felipe Campoverde and authorized contributors under formal agreement.

Unauthorized commercial use may result in legal action.

## MIT License

Copyright (c) [2025]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Note: Commercial use restrictions stated in the README take precedence over the default MIT License terms. Commercial licensing requires written permission from Felipe Campoverde.
