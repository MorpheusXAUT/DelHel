# DelHel

`DelHel` helps VATSIM controllers by automating repetitive delivery duties in EuroScope.

The plugin performs validation of FPLs (**f**light **pl**ans), sets the appropriate SID (**s**tandard **i**nstrument **d**eparture) depending on runway config and changes the cleared flight level to the initial value of the respective SID. If available, NAPs (**n**oise **a**batement **p**rocedures) will be respected during SID assignment.

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
7. Assign the `DelHel / Process FPL` action as the **Left button** or **Right button** action of any of your tag items as desired. Triggering this function processes the selected flightplan using the default settings of `DelHel` (described in more detail in the [Process FPL](#process-fpl) section below)
8. (*Optional*) Assign the `DelHel / Validation menu` action as the **Left button** or **Right button** action of any of your tag items as desired. Triggering this function opens the flightplan validation menu, allowing for more fine-grained processing of the selected flightplan (described in more detail in the [Validation Menu](#validation-menu) section below)
9. Close the departure list settings by clicking **OK**

## Usage

### Basics

By default, `DelHel` only performs some basic validations of flightplans, displaying warnings for potential issues found for each aircraft in your departure list via the [`Flightplan Validation`](#flightplan-validation) tag item. Additionally, flightplan processing and more detailed validations can be triggered manually using the [`Process FPL`](#process-fpl) tag functions.

At the moment, `DelHel` supports the following validations and processing:
- SID validation: ensures FPL has a valid SID fix for the departure airport and runway config as its first waypoint. Upon processing a flightplan, the calculated SID will be confirmed (added to the filed route in the FPL) so any changes to runway configs or different controller setups should have no effect.
- NAP assignment: if available (and [enabled](#toggle-assignment-of-nap-sids)), noise abatement procedure SIDs will be assigned as appropriate based on runway config.
- CFL (**c**leared **f**light **l**evel) validation: verifies correct CFL is set for selected SID. Processing a FPL automatically sets the correct CFL for the calculated SID.
- RWY (**r**un**w**a**y**) validation: displays a warning if a runway assignment has been found in the flightplan as this might influence SID selection. Should no RWY be selected by EuroScope automatically (e.g. the filed SID fix does not have a SID for the active RWYs), `DelHel` will attempt to match the SIDs defined for all RWYs set as active in EuroScope to find an alternative valid departures (as defined in the `airports.json` file).
- Flightplan cleanup: when processing a flightplan, some cleanup will be performed, removing any additional information the pilot might have included before the SID fix (e.g. a SID filed by the pilot), only leaving a valid speed/level group or a runway designation included. This prevents SID assignments filed by pilots from selecting an incorrect runway or procedure by accident.
- INV (**Inv**alid routing): verifies that the filed flightplan contains a valid routing according to the maintained `routing.json` file. Those routes can e.g. be the SAXFRA compulsory routings or special Vatsim Event routings. Each route contains at least an entry point (= last waypoint of the SID) and optional waypoints. If waypoints are maintained, the flightplan has to contain those waypoints in identical sequence in order to pass the routing validation. "DCT"/"DIRECT"/Speed-Altitude-Blocks in the FP are omitted.

Flightplans can be processed manually using the [`Process FPL`](#process-fpl) [tag function](#tag-functions) or automatically by toggling the [automatic processing](#toggle-automatic-processing) setting on.

### Tag items

Tag items are used to display information about flightplans in aircraft lists, such as the departure or arrival list.  
At the moment, `DelHel` only adds one (optional) tag item to EuroScope:

#### Flightplan Validation

The `Flightplan Validation` tag item displays the result of the flightplan validation performed by `DelHel` for each aircraft. It contains any warnings or errors the plugin encounters while checking the FPL and is also the suggested column to interact with the `DelHel` [tag functions](#tag-functions).  
To save space, only one indication will be displayed at a time in the order they are encountered during processing, so you might see multiple warnings for a single flightplan while handling it.

The following indications are available:

##### `ADEP`

Error, displayed in red.  
The plugin did not find the departure airport of the aircraft in its [airport config](#airport-config) and thus cannot perform any validations or processing. This might indicate an incorrectly filed departure ICAO code or an airport currently not supported by `DelHel`.

##### `CFL`

Info, no/default color.  
Indicates the current CFL set does not equal the initial CFL defined for the SID assigned. This most likely indicates the flightplan has not been processed yet or the aircraft was assigned a cleared flight level that deviates from the default.

##### `RFL`

Caution, orange color.  
Indicates the RFL filed in the FPL is below the initial CFL defined for the SID assigned. While this does not always indicate an error, caution is advised so minimum altitudes or other restrictions are not violated.  
This warning is disabled by default and can be toggled using the [Toggle RFL below inital CFL](#toggle-rfl-below-initial-cfl-check) chat command.

##### `RWY`

Info, no/default color.  
Indicates a runway designation (e.g. `LOWW/16`) has been found in the flightplan route. This is most likely the result of a manual runway assignment by a controller (via the TopSky plugin), but could also have been filed by a pilot, thus reminding you to ensure the correct RWY (and therefore SID) has been assigned.

Info, green color.  
Indicates a runway designation (e.g. `LOWW/16`) has been found in the flightplan route of an already processed FPL. This is most likely the result of a manual runway assignment by a controller (via the TopSky plugin), but could also have been filed by a pilot, thus reminding you to ensure the correct RWY (and therefore SID) has been assigned.

Error, displayed in red.  
Indicates an error with the runway config, as no departure runway could be assigned by EuroScope. Ensure you have at least one departure runway set as active in your EuroScope runway dialog. Alternatively this could indicate an error in the FPL route as no suitable RWY could be found.

##### `SID`

Error, displayed in red.  
Indicates no valid SID fix has been found in the flightplan, the pilot has either filed an invalid FPL route without a proper departure waypoint set or the route is malformed and no proper fix could be detected. Open the flightplan and check the route, adapting it manually if required, or ask the pilot to file a new flightplan with a proper route.

##### `VFR`

Info, no/default color.  
Indicates pilot filed a VFR flightplan, so little to no validations can be performed. This just serves as an additional reminder about VFR flights.

Info, green color.  
Indicates pilot filed a processed VFR flightplan, so little to no validations can be performed. This just serves as an additional reminder about VFR flights.

##### `MIN`

Caution, orange color.  
Indicates, that the Requested Flight Level (RFL) is below the minima for the filed route. This warning will only be displayed if [checking of min and max RFLs](#toggle-checking-of-min-and-max-rfls) is enabled (default off).  
The altitudes are maintained in the routing.json and can be disregarded by the controller if necessary (e.g. depeding on active/inactive military areas along the route).

##### `MAX`

Caution, orange color.  
Indicates, that the Requested Flight Level (RFL) is above the maxima for the filed route. This warning will only be displayed if [checking of min and max RFLs](#toggle-checking-of-min-and-max-rfls) is enabled (default off).  
The altitudes are maintained in the routing.json and can be disregarded by the controller if necessary (e.g. depeding on active/inactive military areas along the route).


##### `INV`

Caution, orange color.  
Indicates, that the filed route is not valid according to the maintained routings (routing.json). Those routings are e.g. the compulsory SAXRFRA routings for LOWW-departures or in case of a Vatsim Event.
Indication becomes active as soon as the flightplan has been processed (SID is set and a valid SID exit point is in the FP).

##### `OK`

Info, green color.  
Indicates a flightplan has been processed and no validation errors or warnings have been found.

### Tag functions

Tag functions are used to trigger plugin functionality via a flightplan tag in aircraft lists, such as the departure or arrival list.  
At the moment, `DelHel` adds several functions for processing FPLs which can be added as an action to any tag item desirable (although using them with the [`Flightplan Validation`](#flightplan-validation) tag item is recommended):

#### Process FPL

Processes the selected flightplan using the default global plugin settings, checking for a valid SID fix (as defined in the [airport config](#airport-config)) while cleaning up the route (removing everything filed before the SID fix aside from speed/level groups and runway designations).  
Based on the SID fix and selected runway config, an appropriate SID (or NAP SID, if available/[enabled](#toggle-assignment-of-nap-sids)) will be picked from the airport config and the respective CFL assigned.  
Since VFR flightplans contain no SID and often have just a very basic (if any) route available, the number of validations and automatic changes available is quite limited. At the moment of writing, `DelHel` will only assign a default altitude (standard traffic pattern altitude, airport elevation + 1000ft, rounded to the nearest 500ft) for a VFR flightplan and skip all other processing.  
Note: using this function does **not** assign a squawk since TopSky does not expose its squawk logic and re-implementing the exact same assignment logic is outside the scope of this project.  
This is the default action of `DelHel` as it allows the most flexible and comfortable processing of FPLs and is thus suggested as a left-click action for the [`Flightplan Validation`](#flightplan-validation) tag item.

#### Process FPL (NAP)

Processes the selected flightplan as described in [Process FPL](#process-fpl) above, however forces using noise abatement procedures (where available), even if the global NAP assignment has been toggled off.  
This action is also available in the [`Validation Menu`](#validation-menu) tag function by default and allows for separate processing of selected flightplans without changing the global NAP setting.

#### Process FPL (non-NAP)

Processes the selected flightplan as described in [Process FPL](#process-fpl) above, however forces using "regular" (non-NAP) SIDs, even if the global NAP assignment has been toggled on.  
This action is also available in the [`Validation Menu`](#validation-menu) tag function by default and allows for separate processing of selected flightplans without changing the global NAP setting.

#### Validation Menu

Opens a selection menu containing advanced options regarding flightplan validation, currently only the [`Process FPL (NAP)`](#process-fpl-nap) and [`Process FPL (non-NAP)`](#process-fpl-non-nap) functions. As more functionality is added, additional entries will be included in this menu.  
This is the default secondary action of `DelHel` as it allows more fine-grained processing of FPLs and is thus suggested as the right-click action for the [`Flightplan Validation`](#flightplan-validation) tag item.

### Chat commands

Chat commands allow more fine-grained control of `DelHel`'s behavior and settings not available via UI elements. Every chat command is prefixed with `.delhel` and can be entered in every chat channel available. Executing `.delhel` without any additional commands prints the version loaded and a list of commands available.

#### Toggle automatic processing

`.delhel auto`

Toggles automatic processing of flightplans.

Once enabled, `DelHel` will automatically process flightplans roughly every 5 seconds using the default processing settings as described in [Process FPL](#process-fpl) above. The plugin will only consider aircraft below a certain altitude threshold (current hard-coded to 5000ft) as already airborne pilots will most likely not receive a new clearance anyways.  
**Attention**: use this setting with caution, there is currently no limit to active airports or regard taken for controllers "below" you - `DelHel` will process flightplans for all aircraft it can "see" and has a valid departure [airport config](#airport-config) for.

This setting will always reset to its disabled state on every startup to avoid issues after connecting to a new session.

#### Toggle debug logging

`.delhel debug`

Toggles debug logging, displaying more messages about the internal state and flightplan processing.

This setting will be saved to the EuroScope settings upon exit.

#### Toggle assignment of NAP SIDs

`.delhel nap`

Toggles automatic assignment of NAP SIDs for the [default processing](#process-fpl) functionality.

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

#### Toggle RFL below initial CFL check

`.delhel rflblw`

Toggles the flightplan check for RFLs below the initial CFL for SIDs.

If enabled, `DelHel` will check the RFL of a flightplan and display a [caution](#rfl) if the filed final level is below the initial CFL for the SID assigned.  
If disabled (default setting), `DelHel` will display no indication and silently assign the lower RFL as the CFL while processing.

This setting will be saved to the EuroScope settings upon exit.

#### Toggle logging of min and max RFLs

`.delhel logminmaxrfl`

Toggles logging of min and max RFLs for predefined routings during flightplan processing.

If enabled, `DelHel` will display the min/max value triggering a warning during flightplan processing, informing you about the limit defined in the routing config.  
If disabled (default setting), `DelHel` will only display the min/max RFL values if the [debug mode](#toggle-debug-logging) is enabled.

This setting will be saved to the EuroScope settings upon exit.

#### Toggle checking of min and max RFLs

`.delhel minmaxrfl`

Toggles checking of min and max RFLs for predefined routings during flightplan processing.

If enabled, `DelHel` will verify the min/max value of a predefined route during flightplan processing, displaying a warning if the filed RFL is below or above the limit defined in the routing config.  
If disabled (default setting), `DelHel` will not display the [`MIN`](#min)/[`MAX`](#max) warnings.

This setting will be saved to the EuroScope settings upon exit.

#### Toggle flashing of DelHel messages

`.delhel flash`

Toggles flashing of unread message indicator for messages in the DelHel group. Note that, once disabled, all `DelHel` messages will continue to flash until you have restarted EuroScope (saving your plugin settings). This unfortunately seems to be a EuroScope limitation we cannot work around.

If enabled, messages sent to the `DelHel` group will have a flashing unread indiciator.  
If disabled (default setting), unread messages will only light up once, solidly (only applies after EuroScope has been restarted).

This setting will be saved to the EuroScope settings upon exit.

### Airport config

`DelHel` uses its airport config, stored in the `airports.json` file in the same directory as the `DelHel.dll` plugin DLL, to retrieve most of its configuration for validations and flightplan processing.  
This repository contains the default airport config for `DelHel`, although you might choose to adapt it to your vACC's needs and remove entries that are not relevant to you. Please keep in mind the same structure must still be provided as the plugin will fail to parse its config otherwise.

The `airports.json` file is a [JSON](https://www.json.org/) file containing a top-level object with the departure airport's ICAO code in uppercase as keys and `Airport` objects as values.

#### `Airport` object

Key       | Type     | Description                                                                                   | Required
----------|----------|-----------------------------------------------------------------------------------------------|---------
elevation | `int`    | Airport elevation in feet (ft)                                                                | Yes
sids      | `object` | Object with SID fixes as keys and `SID` objects as values, contains SIDs available at airport | Yes

#### `SID` object

Key  | Type     | Description                                                                              | Required
-----|----------|------------------------------------------------------------------------------------------|---------
cfl  | `int`    | Initial CFL for SID in feet (ft)                                                         | Yes
rwys | `object` | Object with RWYs as keys and `RWY` objects as values, contains RWYs SID is available for | Yes

#### `RWY` object

Key  | Type     | Description                                                                              | Required
-----|----------|------------------------------------------------------------------------------------------|---------
dep  | `string` | Full name of SID for RWY                                                                 | Yes
nap  | `string` | Full name of NAP for RWY (if available)                                                  | No
prio | `int`    | Priority of RWY for SID if no RWY is assigned by EuroScope (higher number = higher prio) | No

### Routing config
All mandatory routings are stored in the `routing.json`file in the same directory as the `DelHel.dll` plugin. Within this file, you can specify routings with optional waypoints, and the corresponding altitudes (min/max cruise altitude for this route).

Please make sure to follow the file structure, and checking the edited file with a [validator](https://jsonformatter.curiousconcept.com/).

Root object contains the departure ICAO code in (4-letter) uppercase as key and a `entry` object as value.

#### `entry` object
Key    | Type     | Description                                                                 | Required
-------|----------|-----------------------------------------------------------------------------|---------
name   | `string` | Describes the route entry waypoint, which is equal to the last SID-waypoint | Yes
routes | `array`  | Storage for more specified routes, e.g. destinations, waypoints, etc.       | Yes

#### `routes` array
The routes array is filled with route objects including the following data:

Key       | Type     | Description                                                                       | Required
----------|----------|-----------------------------------------------------------------------------------|---------
icao      | `string` | Describes the route entry waypoint, which is equal to the last SID-waypoint       | Yes
maxlvl    | `int`    | Maximum allowed cruise altitude in flightlevel (= feet/100) for this route        | Yes
minlvl    | `int`    | Minimum allowed cruise altitude in flightlevel (= feet/100) for this route        | Yes
waypoints | `array`  | Enter all succeeding waypoints after the SID as strings, optional, can left blank | No

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