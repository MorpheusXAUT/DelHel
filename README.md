# DelHel

`DelHel` helps VATSIM controllers by automating repetitive delivery duties in EuroScope.

The plugin performs validation of flightplans, sets the appropriate SID (**s**tandard **i**nstrument **d**eparture) depending on runway config and changes the cleared flight level to the initial value of the respective SID. If available, NAPs (**n**oise **a**batement **p**rocedures) will be respected during SID assignment.

## Table of Contents

- [Getting started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
- [Usage](#usage)
  - [Basics](#basics)
  - [Tag items](#tag-items)
  - [Tag functions](#tag-functions)
  - [Chat commands](#chat-commands)
  - [Airport config](#airport-config)
  - [Flightplan processing](#flightplan-processing)
- [Contributing](#contributing)
  - [Development setup](#development-setup)
- [License](#license)

## Getting started

### Prerequisites

Since `DelHel` was developed as an EuroScope plugin, it requires a working installation [EuroScope](https://euroscope.hu/). The initial development was started using EuroScope version [`v3.2.1.25`](https://www.euroscope.hu/wp/2020/06/28/v3-2-1-25/), although the plugin should most likely also work fine with previous and later versions. As development continues, compatibility to the latest **beta** versions of EuroScope will be maintained as long as possible and feasible.

### Installation

1. Download the latest release (`DelHel.zip`) of `DelHel` from the [**Releases**](https://github.com/MorpheusXAUT/DelHel/releases/latest) section of this repository
2. Extract `DelHel.dll` and `airports.json` and place them **in the same folder** (most likely somewhere inside your EuroScope sectorfile/profile setup, where other plugins are already set up)
3. Start EuroScope and open the **Plug-ins** dialog in the settings menu (**OTHER SET**)
![Plug-ins dialog](https://i.imgur.com/SrVtRp9.png)
4. **Load** the plugin by selecting the `DelHel.dll` you extracted and ensure the proper version is displayed
![Load plugin](https://i.imgur.com/y6koC4g.png)
`DelHel` will also confirm successful initialisation by logging its version to the **Messages** chat:
`[08:34:10] DelHel: Version 0.0.1 loaded.`
5. Close the plugin dialog and open the departure list columns setup dialog (small **S** at the left side of your departure list)
![Departure list columns setup dialog](https://i.imgur.com/MvFYkkh.png)
6. (*Optional*) Add the **Flightplan Validation** column to your departure list by clicking **Add Item** and selecting the `DelHel / Flightplan Validation` **Tag Item type**. Pick a **Header name** and set a **Width** of 4 or greater. This column will display warnings and the status of each flightplan processed by DelHel, but is not strictly required for the plugin to function
7. Assign the `DelHel / Process FPL` action as the **Left button** or **Right button** action of any of your tag items as desired. Triggering this function processes the selected flightplan using the default settings of `DelHel` (described in more detail in the [Default processing](#default-processing) section below)
8. (*Optional*) Assign the `DelHel / Validation menu` action as the **Left button** or **Right button** action of any of your tag items as desired. Triggering this function opens the flightplan validation menu, allowing for more fine-grained processing of the selected flightplan (described in more detail in the [Selected processing](#selected-processing) section below)
9. Close the departure list settings by clicking **OK**

## Usage

### Basics

### Tag items

#### Flightplan Validation

### Tag functions

#### Process FPL

#### Process FPL (NAP)

#### Process FPL (non-NAP)

#### Validation Menu

### Chat commands

Chat commands allow more fine-grained control of `DelHel`'s behavior and settings not available via UI elements. Every chat command is prefixed with `.delhel` and can be entered in every chat channel available. Executing `.delhel` without any additional commands prints the version loaded and a list of commands available.

#### Toggle automatic processing

`.delhel auto`

Toggles automatic processing of flightplans.

Once enabled, `DelHel` will automatically process flightplans roughly every 5 seconds using the default processing settings as described in [Default processing](#default-processing) below. The plugin will only consider aircraft below a certain altitude threshold (current hard-coded to 5000ft) as already airborne pilots will most likely not receive a new clearance anyways.  
**Attention**: use this setting with caution, there is currently no limit to active airports or regard taken for controllers "below" you - `DelHel` will process flightplans for all aircraft it can "see" and has a valid departure [airport config](#airport-config) for.

This setting will always reset to its disabled state on every startup to avoid issues after connecting to a new session.

#### Toggle debug logging

`.delhel debug`

Toggles debug logging, displaying more messages about the internal state and flightplan processing.

This setting will be saved to the EuroScope settings upon exit.

#### Toggle assignment of NAP SIDs

`.delhel nap`

Toggles automatic assignment of NAP SIDs for the [default processing](#default-processing) functionality.

This setting will be saved to the EuroScope settings upon exit.

#### Reload airport config

`.delhel reload`

Reloads the [airport config](#airport-config) from disk, allowing for a new config to be loaded without having to completely restart EuroScope or unloading/reloading the plugin.  
Note: this does not reset the processed state for the [automatic processing](#toggle-automatic-processing) features, so aircraft previously handled will not received an updated SID based on the new config. To reset the list of processed aircraft and handle every flightplan again, see [Reset plugin state](#reset-plugin-state).

#### Reset plugin state

`.delhel reset`

Resets the plugin state to its default values (respecting the saved settings from the EuroScope config), disabling [automatic processing](#toggle-automatic-processing), reloading the [airport config](#airport-config) and clearing the list of processed aircraft.

#### Toggle update checks

`.delhel update`

Toggles the plugin update check upon EuroScope startup.

If enabled, `DelHel` will check this repository for newer releases of the plugin, displaying a message should an update be available.

This setting will be saved to the EuroScope settings upon exit.

### Airport config

### Flightplan processing

#### Default processing

#### Selected processing

## Contributing

If you have a suggestion for the project or encountered an error, please open an [issue](https://github.com/MorpheusXAUT/DelHel/issues) on GitHub. Please provide a summary of your idea or problem, optionally with some logs or screenshots and ways to replicate for the latter.  
The current development state as well as a rough collection of ideas can also be found in the `DelHel` [project board](https://github.com/MorpheusXAUT/DelHel/projects/1) on GitHub.

[Pull requests](https://github.com/MorpheusXAUT/DelHel/pulls) are highly welcome, feel free to extend the plugin's functionality as you see fit and submit a request to this repository to make your changes available to everyone. Please keep in mind this plugin attempts to provide features in a relatively generic way so it can be used by vACCs with different needs - try refraining from "hard-coding" any features that might just apply to a specific airport or vACC.

### Development setup

`DelHel` currently has no external development dependencies aside [Visual Studio](https://visualstudio.microsoft.com/vs/). Initial development started using Visual Studio 2019, although later versions should most likely remain compatible.

To allow for debugging, the project has been configured to launch EuroScope as its debug command. Since your installation path of EuroScope will most likely be different, you **must** set an environment variable `EUROSCOPE_ROOT` to the **directory** EuroScope is installed in (**not** the actual `EuroScope.exe` executable), for instance `E:\EuroScope`.  
Note: triggering a breakpoint seems to cause both EuroScope and Visual Studio to freak out, resulting in high resource usage and slugging mouse movements, thus only being of limited usefulnes. **NEVER** debug your EuroScope plugin using a live connection as halting EuroScope apparently messes with the VATSIM data feed under certain circumstances.

`DelHel` is compiled using Windows SDK Version 10.0 with a platform toolset for Visual Studio 2019 (v142) using the ISO C++17 Standard.

This repository contains all external dependencies used by the project in their respective `include` and `lib` folders:

- `EuroScope`: EuroScope plugin library
- `nlohmann/json`: [JSON for Modern C++](https://github.com/nlohmann/json/) ([v3.9.1](https://github.com/nlohmann/json/releases/tag/v3.9.1), [MIT License](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT)), used for parsing the airport config JSON
- `semver`: [Semantic Versioning C++](https://github.com/Neargye/semver) ([v0.2.2](https://github.com/Neargye/semver/releases/tag/v0.2.2), [MIT License](https://github.com/Neargye/semver/blob/master/LICENSE)), used for version comparison of update check

## License

[MIT License](LICENSE)