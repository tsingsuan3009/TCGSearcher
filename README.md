# TCGSearcher

## Build

Open `TCGSearcher.pro` with Qt Creator, select a Qt 6 kit with the Qt Widgets
and Network modules, then build and run. The project resources are stored in
`resources/`; Qt Creator can generate a fresh build directory, so the existing
`build/` directory is not required.

Card data is downloaded over HTTPS from this repository when a card-search
window opens. An internet connection is required to search cards.