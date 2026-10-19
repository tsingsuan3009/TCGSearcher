# 集换式卡牌搜索工具（TCGSearcher）

TCGSearcher 是一个使用 C++17 和 Qt Widgets 编写的桌面卡牌检索程序，提供简体中文界面。它将游戏王、宝可梦集换式卡牌（PTCG）和《原神》七圣召唤的查询入口放在同一个应用中，便于按卡名、卡片编号或其他关键词查找卡牌，并查看数据源提供的卡片资料。

卡牌目录不保存在项目中，也不依赖某一份固定的 JSON 文件。程序会在打开原神图鉴或搜索宝可梦 TCG、游戏王卡牌时，通过 HTTPS 请求相应的数据网站；数据源发布新卡后，联网查询可以直接使用更新后的目录，无须等待本项目重新生成或提交数据文件。

## 功能

- **游戏王卡牌搜索**：实时查询游戏王百鸽卡查，显示卡片密码、中文名、日文名、英文名、类型、攻击力、守备力和描述。支持按名称、卡片密码或描述关键词搜索；双击结果可查看卡片资料。
- **宝可梦 TCG 卡牌搜索**：实时查询 tcg.mik.moe 的简体中文卡牌目录，显示名称、Yoren Code 和系列卡号；双击结果会再向数据源请求该卡的详情，包括类型、稀有度、规则标记、发行日期、画师、卡牌说明及宝可梦属性。
- **原神七圣召唤搜索**：打开图鉴时，实时获取米游社的角色卡与行动卡目录；可按名称、编号、类别、角色元素及数据源返回的费用等信息筛选，并查看数据源提供的卡牌属性。
- **焕新界面**：主菜单以三张主题卡片进入对应图鉴；搜索窗口采用统一深色控件与按图鉴区分的背景图，表格文字保持高对比度。
- **卡牌规则与帮助**：主界面的“帮助”菜单提供三个游戏的规则说明。
- **窗口工具**：主界面提供清除搜索结果和重置窗口布局等工具。
- **内置界面资源**：按钮图标、背景图片和规则文本编译进 Qt 资源，不依赖构建输出目录中的文件。

## 在线数据源

| 图鉴 | 在线来源 | 请求方式和数据范围 |
| --- | --- | --- |
| 原神七圣召唤 | [米游社角色卡接口](https://api-takumi.mihoyo.com/event/cardsquare/roles) 与 [行动卡接口](https://api-takumi.mihoyo.com/event/cardsquare/actions) | 打开图鉴时分别获取当前角色卡和行动卡目录。 |
| 宝可梦 TCG | [tcg.mik.moe](https://tcg.mik.moe/) | 每次搜索调用卡牌搜索接口；双击卡牌时按系列和卡号获取详细资料。 |
| 游戏王 | [游戏王百鸽卡查](https://ygocdb.com/) | 每次搜索将关键词提交给在线卡查并显示返回结果。 |

以上数据由各自的网站维护，卡牌更新速度、语言、字段完整度和服务可用性取决于对应数据源。程序不维护本地卡牌数据库，也不会在每次发行新卡时要求更新本项目。当前使用的公开接口返回哪些字段，图鉴及详情就展示哪些字段；例如七圣召唤接口提供角色属性和行动卡基础信息，但未必包含完整的卡牌规则文本。

联网请求失败、服务返回错误或响应格式不符合预期时，程序会显示错误提示并记录错误日志。卡牌搜索需要可访问相应数据源的互联网连接；离线时无法查询在线卡牌数据。程序不要求用户提供 API 密钥。

## 构建和运行

### 环境要求

- Qt 6（包括 **Qt Widgets** 和 **Qt Network** 模块）
- 支持 C++17 的编译器
- Qt Creator / qmake
- 运行时能够通过 HTTPS 访问上表中的数据源

仓库中的 `.pro.user` 是本机 Qt Creator 配置，不属于构建所需文件。请在自己的 Qt Creator 中选择已安装的 Qt Kit。

### 使用 Qt Creator

1. 克隆仓库，并在 Qt Creator 中打开 `TCGSearcher.pro`。
2. 选择带有 Qt Widgets、Qt Network 和相应 C++ 编译器的 Qt 6 Kit。
3. 选择“配置项目”，然后构建并运行。

`resources.qrc` 和 `resources/` 中的界面素材均位于源码目录。Qt Creator 会重新生成构建目录及编译产物；项目无需预先存在 `build/` 文件夹，也不需要下载或复制卡牌 JSON。

### 命令行构建（可选）

在已配置 Qt 和编译器环境的终端中，可使用 qmake 生成构建文件并编译：

```text
qmake TCGSearcher.pro
mingw32-make
```

若使用其他编译器，请使用该 Qt Kit 对应的构建工具，而不是 `mingw32-make`。

## 项目结构

```text
CardSearch.cpp/.h       搜索窗口基类、网络请求和通用错误提示
YGOSearch.cpp/.h        游戏王在线搜索与结果详情
PTCGSearch.cpp/.h       宝可梦 TCG 在线搜索与卡片详情
GenshinSearch.cpp/.h    七圣召唤在线目录、筛选和卡片属性
StartMenu.cpp/.h        主菜单、规则说明和窗口工具
resources.qrc           Qt 界面资源清单
resources/              图标、背景图片和规则文本
TCGSearcher.pro         qmake 项目配置
```

## 界面素材与许可

三款图鉴背景使用 Wikimedia Commons 上可再分发的素材；文件随 Qt 资源一同打包，运行时不需要额外联网下载：

| 用途 | 素材与作者 | 许可与来源 |
| --- | --- | --- |
| 游戏王背景 | NASA/ESA/CSA/STScI，《NASA Telescopes Create Colorful 'Craft' From Nearby Nebula (Tarantula Nebula Optical Crop)》 | 公有领域；[Wikimedia Commons 文件页](https://commons.wikimedia.org/wiki/File:NASA_Telescopes_Create_Colorful_%27Craft%27_From_Nearby_Nebula_(Tarantula_Nebula_Optical_Crop).jpg) |
| 宝可梦 TCG 背景 | David Revoy，《Fantasy Landscape》 | [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/)；[Wikimedia Commons 文件页](https://commons.wikimedia.org/wiki/File:2007-01-30_Fantasy-Landscape_by-David-Revoy.jpg) |
| 七圣召唤背景 | Hpjperera84，《Misty mountain of Sri Lanka》 | [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/)；[Wikimedia Commons 文件页](https://commons.wikimedia.org/wiki/File:Misty_mountain_of_Sri_lanka.jpg) |

素材保持原样并注明作者及来源。游戏名称与标识仅用于指明图鉴类别，不表示本项目获得相关权利人背书。

## 版本与数据说明

- 代码、界面资源和规则文本由本仓库进行版本控制。
- 卡牌目录是运行时从上游网站获取的，不再随项目提交静态卡牌 JSON。
- 如需核实卡牌的最新状态、规则或图像版权，请以对应数据源及游戏官方发布内容为准。
- 本项目为独立的社区工具，与米哈游、宝可梦公司、科乐美及上述卡牌数据网站无隶属或官方背书关系。
