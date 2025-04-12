#include "pch.h"

#include "CDelHel.h"

CDelHel* pPlugin;

CDelHel::CDelHel() : EuroScopePlugIn::CPlugIn(
	EuroScopePlugIn::COMPATIBILITY_CODE,
	PLUGIN_NAME,
	PLUGIN_VERSION,
	PLUGIN_AUTHOR,
	PLUGIN_LICENSE
)
{
	std::ostringstream msg;
	msg << "Version " << PLUGIN_VERSION << " loaded.";

	this->LogMessage(msg.str());

	this->RegisterTagItemType("Flightplan Validation", TAG_ITEM_FP_VALIDATION);
	this->RegisterTagItemFunction("Validation menu", TAG_FUNC_VALIDATION_MENU);
	this->RegisterTagItemFunction("Process FPL", TAG_FUNC_PROCESS_FP);
	this->RegisterTagItemFunction("Process FPL (non-NAP)", TAG_FUNC_PROCESS_FP_NON_NAP);
	this->RegisterTagItemFunction("Process FPL (NAP)", TAG_FUNC_PROCESS_FP_NAP);

	this->RegisterDisplayType(PLUGIN_NAME, false, false, false, false);

	this->debug = false;
	this->updateCheck = false;
	this->assignNap = false;
	this->autoProcess = false;
	this->warnRFLBelowCFL = false;
	this->logMinMaxRFL = false;
	this->checkMinMaxRFL = false;
	this->flashOnMessage = false;
	this->topSkyAvailable = false;
	this->ccamsAvailable = false;
	this->preferTopSkySquawkAssignment = false;
	this->customRunwayConfig = "";

	this->LoadSettings();
	this->CheckLoadedPlugins();

	this->ReadAirportConfig();
	this->ReadRoutingConfig();
	this->ReadCustomConfigs();

	if (this->updateCheck) {
		this->latestVersion = std::async(FetchLatestVersion);
	}
}

CDelHel::~CDelHel()
{
}

bool CDelHel::OnCompileCommand(const char* sCommandLine)
{
	std::vector<std::string> args = split(sCommandLine);

	if (starts_with(args[0], ".delhel")) {
		if (args.size() == 1) {
			std::ostringstream msg;
			msg << "Version " << PLUGIN_VERSION << " loaded. Available commands: auto, debug, nap, reload, reset, update, rflblw, logminmaxrfl, minmaxrfl, flash, ccams, rwycfg";

			this->LogMessage(msg.str());

			return true;
		}

		if (args[1] == "debug") {
			if (this->debug) {
				this->LogMessage("Disabling debug mode", "Debug");
			}
			else {
				this->LogMessage("Enabling debug mode", "Debug");
			}

			this->debug = !this->debug;

			this->SaveSettings();

			return true;
		}
		else if (args[1] == "update") {
			if (this->updateCheck) {
				this->LogMessage("Disabling update check", "Update");
			}
			else {
				this->LogMessage("Enabling update check", "Update");
			}

			this->updateCheck = !this->updateCheck;

			this->SaveSettings();

			return true;
		}
		else if (args[1] == "nap") {
			if (this->assignNap) {
				this->LogMessage("No longer assigning NAP SIDs", "Config");
			}
			else {
				this->LogMessage("Assigning NAP SIDs", "Config");
			}

			this->assignNap = !this->assignNap;

			this->SaveSettings();

			return true;
		}
		else if (args[1] == "auto") {
			if (this->autoProcess) {
				this->LogMessage("No longer automatically processing flightplans", "Config");
			}
			else {
				this->LogMessage("Automatically processing flightplans", "Config");
			}

			this->autoProcess = !this->autoProcess;

			return true;
		}
		else if (args[1] == "reload") {
			this->LogMessage("Reloading airport config", "Config");

			this->airports.clear();
			this->ReadAirportConfig();
			this->ReadRoutingConfig();

			return true;
		}
		else if (args[1] == "reset") {
			this->LogMessage("Resetting plugin state", "Config");

			this->autoProcess = false;
			this->processed.clear();
			this->airports.clear();
			this->ReadAirportConfig();
			this->ReadRoutingConfig();

			return true;
		}
		else if (args[1] == "rflblw") {
			if (this->warnRFLBelowCFL) {
				this->LogMessage("No longer displaying warnings for RFLs below inital CFLs for SIDs", "Config");
			}
			else {
				this->LogMessage("Displaying warnings for RFLs below inital CFLs for SIDs", "Config");
			}

			this->warnRFLBelowCFL = !this->warnRFLBelowCFL;

			this->SaveSettings();

			return true;
		}
		else if (args[1] == "logminmaxrfl") {
			if (this->logMinMaxRFL) {
				this->LogMessage("No longer logging min and max RFLs for predefined routings", "Config");
			}
			else {
				this->LogMessage("Logging min and max RFLs for predefined routings", "Config");
			}

			this->logMinMaxRFL = !this->logMinMaxRFL;

			this->SaveSettings();

			return true;
		}
		else if (args[1] == "minmaxrfl") {
			if (this->checkMinMaxRFL) {
				this->LogMessage("No longer checking min and max RFLs for predefined routings", "Config");
			}
			else {
				this->LogMessage("Checking min and max RFLs for predefined routings", "Config");
			}

			this->checkMinMaxRFL = !this->checkMinMaxRFL;

			this->SaveSettings();

			return true;
		}
		else if (args[1] == "flash") {
			if (this->flashOnMessage) {
				this->LogMessage("No longer flashing on DelHel message", "Config");
			}
			else {
				this->LogMessage("Flashing on DelHel message", "Config");
			}

			this->flashOnMessage = !this->flashOnMessage;

			this->SaveSettings();

			return true;
		}
		else if (args[1] == "prefertopsky") {
			if (this->preferTopSkySquawkAssignment) {
				this->LogMessage("No longer preferring TopSky squawk assignment, will use CCAMS if loaded", "Config");
			}
			else {
				this->LogMessage("Preferring TopSky squawk assignment, even if CCAMS is loaded", "Config");
			}

			this->preferTopSkySquawkAssignment = !this->preferTopSkySquawkAssignment;

			this->SaveSettings();
			this->CheckLoadedPlugins();

			return true;
		}
		else if (args[1] == "rwycfg")
		{
			if (args.size() < 3)
			{
				std::ostringstream msg;
				msg << "Select custom runway config to apply. Currently active: " << (this->customRunwayConfig.empty() ? "none" : this->customRunwayConfig) << ". Available values : none(disables custom config)";
				for (auto& [name, config] : this->runwayConfigs)
				{
					msg << ", " << name;
				}
				this->LogMessage(msg.str(), "Config");
				return true;
			}

			std::string cfg = args[2];
			to_upper(cfg);
			if (cfg == "NONE")
			{
				this->LogMessage("No longer using custom runway config", "Config");
				this->customRunwayConfig = "";

				return true;
			}

			auto config = this->runwayConfigs.find(cfg);
			if (config == this->runwayConfigs.end())
			{
				this->LogMessage("Invalid custom runway config specified", "Config");
			}
			else
			{
				this->LogMessage("Switched to custom runway config: " + cfg, "Config");
				this->customRunwayConfig = cfg;
			}

			return true;
		}
	}

	return false;
}

void CDelHel::OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	if (!FlightPlan.IsValid()) {
		return;
	}

	switch (ItemCode) {
	case TAG_ITEM_FP_VALIDATION:
		validation res = this->ProcessFlightPlan(FlightPlan, this->assignNap, true);

		if (res.valid && std::find(this->processed.begin(), this->processed.end(), FlightPlan.GetCallsign()) != this->processed.end()) {
			if (res.tag.empty()) {
				strcpy_s(sItemString, 16, "OK");
			}
			else
			{
				strcpy_s(sItemString, 16, res.tag.c_str());
			}

			*pColorCode = EuroScopePlugIn::TAG_COLOR_RGB_DEFINED;

			if (res.color == TAG_COLOR_NONE) {
				*pRGB = TAG_COLOR_GREEN;
			}
			else {
				*pRGB = res.color;
			}
		}
		else
		{
			strcpy_s(sItemString, 16, res.tag.c_str());

			if (res.color != TAG_COLOR_NONE) {
				*pColorCode = EuroScopePlugIn::TAG_COLOR_RGB_DEFINED;
				*pRGB = res.color;
			}
		}
		
		break;
	}
}

void CDelHel::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area)
{
	EuroScopePlugIn::CFlightPlan fp = this->FlightPlanSelectASEL();
	if (!fp.IsValid()) {
		return;
	}

	switch (FunctionId) {
	case TAG_FUNC_VALIDATION_MENU:
		this->OpenPopupList(Area, "Validation", 1);
		this->AddPopupListElement("Process FPL (NAP)", NULL, TAG_FUNC_PROCESS_FP_NAP, false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, false, false);
		this->AddPopupListElement("Process FPL (non-NAP)", NULL, TAG_FUNC_PROCESS_FP_NON_NAP, false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, false, false);
		break;
	case TAG_FUNC_PROCESS_FP:
		this->ProcessFlightPlan(fp, this->assignNap);
		break;
	case TAG_FUNC_PROCESS_FP_NAP:
		this->ProcessFlightPlan(fp, true);
		break;
	case TAG_FUNC_PROCESS_FP_NON_NAP:
		this->ProcessFlightPlan(fp, false);
	}
}

void CDelHel::OnTimer(int Counter)
{
	if (this->updateCheck && this->latestVersion.valid() && this->latestVersion.wait_for(0ms) == std::future_status::ready) {
		this->CheckForUpdate();
	}
	if (this->autoProcess && Counter % 5 == 0) {
		this->AutoProcessFlightPlans();
	}
}

void CDelHel::OnFlightPlanDisconnect(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	this->processed.erase(std::remove(this->processed.begin(), this->processed.end(), FlightPlan.GetCallsign()), this->processed.end());
}

void CDelHel::OnAirportRunwayActivityChanged()
{
	this->UpdateActiveAirports();
}

EuroScopePlugIn::CRadarScreen* CDelHel::OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
	this->radarScreen = new RadarScreen();
	return this->radarScreen;
}

void CDelHel::LoadSettings()
{
	const char* settings = this->GetDataFromSettings(PLUGIN_NAME);
	if (settings) {
		std::vector<std::string> splitSettings = split(settings, SETTINGS_DELIMITER);

		if (splitSettings.size() < 8) {
			this->LogMessage("Invalid saved settings found, reverting to default.");

			this->SaveSettings();

			return;
		}

		std::istringstream(splitSettings[0]) >> this->debug;
		std::istringstream(splitSettings[1]) >> this->updateCheck;
		std::istringstream(splitSettings[2]) >> this->assignNap;
		std::istringstream(splitSettings[3]) >> this->warnRFLBelowCFL;
		std::istringstream(splitSettings[4]) >> this->logMinMaxRFL;
		std::istringstream(splitSettings[5]) >> this->checkMinMaxRFL;
		std::istringstream(splitSettings[6]) >> this->flashOnMessage;
		std::istringstream(splitSettings[7]) >> this->preferTopSkySquawkAssignment;

		this->LogDebugMessage("Successfully loaded settings.");
	}
	else {
		this->LogMessage("No saved settings found, using defaults.");
	}
}

void CDelHel::SaveSettings()
{
	std::ostringstream ss;
	ss << this->debug << SETTINGS_DELIMITER
		<< this->updateCheck << SETTINGS_DELIMITER
		<< this->assignNap << SETTINGS_DELIMITER
		<< this->warnRFLBelowCFL << SETTINGS_DELIMITER
		<< this->logMinMaxRFL << SETTINGS_DELIMITER
		<< this->checkMinMaxRFL << SETTINGS_DELIMITER
		<< this->flashOnMessage << SETTINGS_DELIMITER
		<< this->preferTopSkySquawkAssignment;

	this->SaveDataToSettings(PLUGIN_NAME, "DelHel settings", ss.str().c_str());
}

void CDelHel::ReadCustomConfigs()
{
	json j;

	try {
		std::filesystem::path base2(GetPluginDirectory());
		base2.append("customconfigs.json");

		// Custom runway config file is optional, skip reading if it doesn't exist
		if (!std::filesystem::exists(base2))
		{
			this->LogDebugMessage("No custom runway config file found, skipping loading.", "Config");
			return;
		}

		std::ifstream ifs(base2.c_str());

		j = json::parse(ifs);
	}
	catch (std::exception e)
	{
		this->LogMessage("Failed to read custom runway configs json. Error: " + std::string(e.what()), "Config");
		return;
	}

	for (auto& [configName, jconfig] : j.items())
	{
		std::string configUpper = configName;
		to_upper(configUpper);
		std::string def = jconfig.value<std::string>("def", "");
		if (def == "")
		{
			this->LogMessage("Missing default runway for \"" + configUpper + "\".", "Config");
			continue;
		}
		rwy_config c{
			def
		};

		json sids;
		try
		{
			sids = jconfig.at("sids");
		}
		catch (std::exception e)
		{
			this->LogMessage("Failed to get SIDs for runway config \"" + configUpper + "\". Error: " + std::string(e.what()), "Config");
			continue;
		}

		for (auto& [wp, jwp] : sids.items())
		{
			rwy_config_sid cs{
				wp
			};

			json rwys;
			try
			{
				rwys = jwp.at("rwys");
			}
			catch (std::exception e)
			{
				this->LogMessage("Failed to get RWYs for WP \"" + wp + "\" in \"" + configUpper + "\". Error: " + std::string(e.what()), "Config");
				continue;
			}

			for (auto& [rwy, jrwy] : rwys.items())
			{
				int prio = jrwy.value<int>("prio", 0);
				cs.rwyPrio.emplace(rwy, prio);
			}

			c.sids.emplace(wp, cs);
		}

		this->runwayConfigs.emplace(configUpper, c);
	}

	this->LogDebugMessage("Loaded " + std::to_string(this->runwayConfigs.size()) + " custom runway config(s).", "Config");
}

void CDelHel::ReadRoutingConfig()
{
	json j;

	try {
		std::filesystem::path base2(GetPluginDirectory());
		base2.append("routing.json");

		std::ifstream ifs(base2.c_str());

		j = json::parse(ifs);
	}
	catch (std::exception e)
	{
		this->LogMessage("Failed to read routing config. Error: " + std::string(e.what()), "Config");
		return;
	}

	for (auto itair = this->airports.begin(); itair != this->airports.end(); itair++) {

		for (auto& obj : j.items()) {		//iterator on outer json object -> key = departure icao
			if (obj.key() == itair->second.icao) {	//if departure icao has already been read in by AirportConfig

				try {
					for (auto& el : obj.value()["entry"].items()) {	//iterator on routes items

						for (auto& in_el : el.value()["routes"].items()) {

							routing ro{
								obj.key(),
								in_el.value()["icao"],
								in_el.value()["maxlvl"],
								in_el.value()["minlvl"],
								{}
							};

							ro.waypts.push_back(el.value()["name"]);	// add entry-point = SID exit as first waypoint of route

							for (auto& inner_el : in_el.value()["waypoints"].items()) {	// add route waypoints
								ro.waypts.push_back(inner_el.value());
							}

							itair->second.validroutes.push_back(ro);	//add routing to airports

							std::string check = "ADEP:" + ro.adep + "-ADEST:" + ro.adest + "-MAX:" + std::to_string(ro.maxlvl) + "-MIN:" + std::to_string(ro.minlvl) + "-#wpts:" + std::to_string(ro.waypts.size());
							this->LogDebugMessage("New Routing added: " + check, "Config");
						}
					}
				}
				catch (std::exception e) {
					this->LogMessage("Failed to read routing config for " + itair->second.icao + "| Error: " + std::string(e.what()), "Config");
					return;
				}
				this->LogDebugMessage("Routing for departure Airport " + itair->second.icao + " has been added.", "Config");
			}

		}
	}
}

void CDelHel::ReadAirportConfig()
{
	json j;
	try {
		std::filesystem::path base(GetPluginDirectory());
		base.append("airports.json");

		std::ifstream ifs(base.c_str());

		j = json::parse(ifs);
	}
	catch (std::exception e)
	{
		this->LogMessage("Failed to read airport config. Error: " + std::string(e.what()), "Config");
		return;
	}

	for (auto& [icao, jap] : j.items()) {
		airport ap{
			icao, // icao
			jap.value<int>("elevation", 0), // elevation
			false, // active
		};

		json jss;
		try {
			jss = jap.at("sids");
		}
		catch (std::exception e)
		{
			this->LogMessage("Failed to get SIDs for airport \"" + icao + "\". Error: " + std::string(e.what()), "Config");
			continue;
		}

		for (auto& [wp, js] : jss.items()) {
			sid s{
				wp, // wp
				js.value<int>("cfl", 0) // cfl
			};

			json jrwys;
			try {
				jrwys = js.at("rwys");
			}
			catch (std::exception e)
			{
				this->LogMessage("Failed to get RWYs for SID \"" + wp + "\" for airport \"" + icao + "\". Error: " + std::string(e.what()), "Config");
				continue;
			}

			std::ostringstream rrs;
			rrs << icao << "\\/(";
			for (auto it = jrwys.items().begin(); it != jrwys.items().end(); ++it) {
				sidinfo si{
					it.key(), // rwy
					it.value().value<std::string>("dep", ""), // dep
					it.value().value<std::string>("nap", ""), // nap
					it.value().value<int>("prio", 0) // prio
				};

				s.rwys.emplace(si.rwy, si);
				ap.rwys.emplace(si.rwy, false);

				rrs << si.rwy;
				if (std::next(it) != jrwys.items().end()) {
					rrs << '|';
				}
			}
			rrs << ')';

			ap.rwy_regex = std::regex(rrs.str(), std::regex_constants::ECMAScript);

			ap.sids.emplace(wp, s);
		}

		this->airports.emplace(icao, ap);
	}

	this->UpdateActiveAirports();
}

void CDelHel::UpdateActiveAirports()
{
	this->SelectActiveSectorfile();
	for (auto sfe = this->SectorFileElementSelectFirst(EuroScopePlugIn::SECTOR_ELEMENT_RUNWAY); sfe.IsValid(); sfe = this->SectorFileElementSelectNext(sfe, EuroScopePlugIn::SECTOR_ELEMENT_RUNWAY)) {
		std::string ap = trim(sfe.GetAirportName());
		to_upper(ap);

		auto ait = this->airports.find(ap);
		if (ait == this->airports.end()) {
			continue;
		}

		std::string rwy = trim(sfe.GetRunwayName(0));
		to_upper(rwy);

		auto rit = ait->second.rwys.find(rwy);
		if (rit != ait->second.rwys.end()) {
			rit->second = sfe.IsElementActive(true, 0);
		}

		rwy = trim(sfe.GetRunwayName(1));
		to_upper(rwy);

		rit = ait->second.rwys.find(rwy);
		if (rit != ait->second.rwys.end()) {
			rit->second = sfe.IsElementActive(true, 1);
		}

		if (!ait->second.active) {
			ait->second.active = sfe.IsElementActive(true, 0) || sfe.IsElementActive(true, 1);
		}
	}
}


validation CDelHel::ProcessFlightPlan(EuroScopePlugIn::CFlightPlan& fp, bool nap, bool validateOnly)
{
	validation res{
		true, // valid
		"", // tag
		TAG_COLOR_NONE // color
	};
	std::string cs = fp.GetCallsign();

	if (!validateOnly) {
		if (nap) {
			this->LogDebugMessage("Processing flightplan using noise abatement procedures", cs);
		}
		else {
			this->LogDebugMessage("Processing flightplan", cs);
		}
	}

	EuroScopePlugIn::CFlightPlanData fpd = fp.GetFlightPlanData();

	std::string dep = fpd.GetOrigin();
	to_upper(dep);

	std::string arr = fpd.GetDestination();
	to_upper(arr);

	auto ait = this->airports.find(dep);
	if (ait == this->airports.end()) {
		if (!validateOnly) {
			this->LogDebugMessage("Failed to process flightplan, did not find departure airport \"" + dep + "\" in airport config", cs);
		}

		res.valid = false;
		res.tag = "ADEP";
		res.color = TAG_COLOR_RED;

		return res;
	}

	airport ap = ait->second;
	EuroScopePlugIn::CFlightPlanControllerAssignedData cad = fp.GetControllerAssignedData();

	if (strcmp(fpd.GetPlanType(), "V") == 0 || strcmp(fpd.GetPlanType(), "Z") == 0) {
		if (!validateOnly) {
			if (!cad.SetClearedAltitude(round_to_closest(ap.elevation + VFR_TRAFFIC_PATTERN_ALTITUDE, 500))) {
				this->LogMessage("Failed to process VFR flightplan, cannot set cleared flightlevel", cs);
				return res;
			}

			if (this->radarScreen != nullptr && this->ccamsAvailable && !this->preferTopSkySquawkAssignment) {
				this->radarScreen->StartTagFunction(cs.c_str(), nullptr, 0, cs.c_str(), CCAMS_PLUGIN_NAME, CCAMS_TAG_FUNC_ASSIGN_SQUAWK_VFR, POINT(), RECT());
				this->LogDebugMessage("Triggered automatic VFR squawk assignment via CCAMS", cs);
			}
			else {
				// TopSky doesn't have a dedicated VFR squawk assignment function. Force hardcoded VFR squawk assignment
				if (!cad.SetSquawk(VFR_SQUAWK)) {
					this->LogDebugMessage("Failed to set VFR squawk", cs);
				}
			}

			this->LogDebugMessage("Skipping processing of VFR flightplan route", cs);

			// Add to list of processed flightplans if not added by auto-processing already
			this->IsFlightPlanProcessed(fp);
		}

		res.tag = "VFR";

		return res;
	}

	std::vector<std::string> route = split(fpd.GetRoute());
	sid sid;

	auto rit = route.begin();
	while (rit != route.end()) {
		if (std::regex_search(*rit, ap.rwy_regex)) {
			++rit;
			res.tag = "RWY";
			continue;
		}
		
		std::map<std::string, ::sid>::iterator sit;
		std::smatch m;
		if (std::regex_search(*rit, m, REGEX_SPEED_LEVEL_GROUP)) {
			// Try to match waypoint of speed/level group in case SID fix already has one assigned
			sit = ap.sids.find(m[1]);
		}
		else {
			// If no other matchers above yield a result, try to match full route part
			sit = ap.sids.find(*rit);
		}

		if (sit != ap.sids.end()) {
			sid = sit->second;
			break;
		}

		rit = route.erase(rit);
	}

	if (sid.wp == "" || route.size() == 0) {
		if (!validateOnly) {
			this->LogMessage("Invalid flightplan, no valid SID waypoint found in route", cs);
		}

		res.valid = false;
		res.tag = "SID";
		res.color = TAG_COLOR_RED;

		return res;
	}

	if (validateOnly) {
		if (this->warnRFLBelowCFL && fp.GetFinalAltitude() < sid.cfl) {
			res.tag = "RFL";
			res.color = TAG_COLOR_ORANGE;

			return res;
		}

		int cfl = cad.GetClearedAltitude();
		// If CFL == RFL, EuroScope returns a CFL of 0 and the RFL value should be consulted. Additionally, CFL 1 and 2 indicate ILS and visual approach clearances respectively.
		if (cfl < 3) {
			// If the RFL is not adapted or confirmed by the controller, cad.GetFinalAltitude() will also return 0. As a last source of CFL info, we need to consider the filed RFL.
			cfl = cad.GetFinalAltitude();
			if (cfl < 3) {
				cfl = fp.GetFinalAltitude();
			}
		}

		// Display a warning if the CFL does not match the initial CFL assigned to the SID. No warning is shown if the RFL is below the CFL for the SID as pilots might request a lower initial climb.
		if (cfl != sid.cfl && (cfl != fp.GetFinalAltitude() || fp.GetFinalAltitude() >= sid.cfl)) {
			res.valid = false;
			res.tag = "CFL";

			return res;
		}
	}
	else {
		if (!fpd.SetRoute(join(route).c_str())) {
			this->LogMessage("Failed to process flightplan, cannot set cleaned route", cs);
			return res;
		}

		if (!fpd.AmendFlightPlan()) {
			this->LogMessage("Failed to process flightplan, cannot amend flightplan after setting cleaned route", cs);
			return res;
		}

		std::map<std::string, sidinfo>::iterator sit{};
		std::string rwy = fpd.GetDepartureRwy();
		if (rwy == "" || !this->customRunwayConfig.empty()) { 
			this->LogDebugMessage("No runway assigned, or override active, attempting to pick first active runway for SID", cs);

			// SIDs can have a priority assigned per runway, allowing for "hierarchy" depending on runway config (as currently possible in ES sectorfiles).
			// If no priority is assigned, the default of 0 will be used and the first active runway will be picked.
			int prio = -1;
			for (auto [r, active] : ap.rwys) {
				if (active) {
					this->LogDebugMessage("Checking active runway " + r, cs);
					auto s = sid.rwys.find(r);
					if (s != sid.rwys.end()) {
						if (!this->customRunwayConfig.empty())
						{
							this->LogDebugMessage("Custom config " + this->customRunwayConfig + " active, check for custom config of SID wp: " + sid.wp, cs);
							if (!this->runwayConfigs[this->customRunwayConfig].sids.empty()) {
								auto customSid = this->runwayConfigs[this->customRunwayConfig].sids.find(sid.wp);
								if (customSid != this->runwayConfigs[this->customRunwayConfig].sids.end())
								{
									this->LogDebugMessage("Found SID wp " + customSid->first, cs);
									if (!customSid->second.rwyPrio.empty()) {
										auto customRwy = customSid->second.rwyPrio.find(r);
										if (customRwy != customSid->second.rwyPrio.end())
										{
											this->LogDebugMessage("Found SID rwy " + customRwy->first, cs);
											if (customRwy->second > prio)
											{
												this->LogDebugMessage("Found and applied custom RWY priority override for config " + this->customRunwayConfig, cs);
												rwy = r;
												prio = customRwy->second;
											}
										}
									}
								}
							}

							if (this->runwayConfigs[this->customRunwayConfig].def == r && 1 > prio)
							{
								this->LogDebugMessage("Found and applied custom default-RWY priority override for config " + this->customRunwayConfig, cs);
								rwy = r;
								prio = 1;
							}
						}
						else if (rwy == "" && s->second.prio > prio) {
							rwy = r;
							prio = sit->second.prio;
						}
					}
				}
			}

			if (rwy == "") {
				this->LogMessage("Failed to process flightplan, no runway assigned", cs);

				res.valid = false;
				res.tag = "RWY";
				res.color = TAG_COLOR_RED;

				return res;
			}

			// TODO display warning once "valid" tag override below is fixed
			/*res.tag = "SID";
			res.color = TAG_COLOR_GREEN;*/
		}
		
		sit = sid.rwys.find(rwy);

		if (sit == sid.rwys.end()) {
			this->LogMessage("Invalid flightplan, no matching SID found for runway", cs);

			res.valid = false;
			res.tag = "SID";
			res.color = TAG_COLOR_RED;

			return res;
		}

		sidinfo sidinfo = sit->second;

		std::ostringstream sssid;
		if (nap && sidinfo.nap != "") {
			this->LogDebugMessage("--> Assigned sid/rwy: " + sidinfo.nap + "/" + rwy, cs);
			sssid << sidinfo.nap;
		}
		else {
			this->LogDebugMessage("--> Assigned sid/rwy: " + sidinfo.dep + "/" + rwy, cs);
			sssid << sidinfo.dep;
		}
		sssid << "/" << rwy;


		route.insert(route.begin(), sssid.str());

		if (!fpd.SetRoute(join(route).c_str())) {
			this->LogMessage("Failed to process flightplan, cannot set route including SID", cs);
			return res;
		}

		if (!fpd.AmendFlightPlan()) {
			this->LogMessage("Failed to process flightplan, cannot amend flightplan after setting route including SID", cs);
			return res;
		}

		int cfl = sid.cfl;
		if (fp.GetFinalAltitude() < sid.cfl) {
			this->LogDebugMessage("Flightplan has RFL below initial CFL for SID, setting RFL", cs);

			cfl = fp.GetFinalAltitude();
		}

		if (!cad.SetClearedAltitude(cfl)) {
			this->LogMessage("Failed to process flightplan, cannot set cleared flightlevel", cs);
			return res;
		}

		std::string assignedSquawk = cad.GetSquawk();
		if (assignedSquawk.empty() || assignedSquawk == "2000") {
			if (this->radarScreen == nullptr) {
				this->LogDebugMessage("Radar screen not initialised, cannot trigger automatic squawk assignment via TopSky or CCAMS", cs);
			}
			else {
				if (this->preferTopSkySquawkAssignment && this->topSkyAvailable) {
					this->radarScreen->StartTagFunction(cs.c_str(), nullptr, 0, cs.c_str(), TOPSKY_PLUGIN_NAME, TOPSKY_TAG_FUNC_ASSIGN_SQUAWK, POINT(), RECT());
					this->LogDebugMessage("Triggered automatic squawk assignment via TopSky", cs);
				}
				else if (this->ccamsAvailable) {
					this->radarScreen->StartTagFunction(cs.c_str(), nullptr, 0, cs.c_str(), CCAMS_PLUGIN_NAME, CCAMS_TAG_FUNC_ASSIGN_SQUAWK_AUTO, POINT(), RECT());
					this->LogDebugMessage("Triggered automatic squawk assignment via CCAMS", cs);
				}
				else if (this->topSkyAvailable) {
					this->radarScreen->StartTagFunction(cs.c_str(), nullptr, 0, cs.c_str(), TOPSKY_PLUGIN_NAME, TOPSKY_TAG_FUNC_ASSIGN_SQUAWK, POINT(), RECT());
					this->LogDebugMessage("Triggered automatic squawk assignment via TopSky", cs);
				}
				else {
					this->LogDebugMessage("Neither TopSky nor CCAMS are loaded, cannot trigger automatic squawk assignment", cs);
				}
			}
		}

		this->LogDebugMessage("Successfully processed flightplan", cs);

		// Add to list of processed flightplans if not added by auto-processing already
		this->IsFlightPlanProcessed(fp);
	}


	if (ap.validroutes.size() != 0) {

		flightplan fpl = flightplan(fp.GetCallsign(), fp.GetExtractedRoute(), fpd.GetRoute()); // create fp for route validation

		bool routecheck = false;
		int count = 0;
		for (auto vait = ap.validroutes.begin(); vait != ap.validroutes.end(); ++vait) {

			routecheck = false;
			auto selsidit = vait->waypts.begin();

			if (*selsidit == sid.wp) {
				if (vait->waypts.size() > 1) {
					try {

						
						count = 0; //counter to disregard previous found waypoints in fpl
						for (auto wyprouit = vait->waypts.begin(); wyprouit != vait->waypts.end(); ++wyprouit) {
							for (auto wypfpl = fpl.route.begin() + count; wypfpl != fpl.route.end(); ++wypfpl) {

								if (wypfpl->airway && wypfpl->name.rfind(*wyprouit) == 0) { // check if waypoint name is part of the airway (e.g. SID)
									
									routecheck = true;
									++count;
									break;
								}
								if (*wyprouit == wypfpl->name) {
									routecheck = true;
									++count;
									break;
								}
								else {
									routecheck = false;
								}
								++count;
							}
							if (!routecheck) {
								break;
							}

						}
					}
					catch (std::exception e) {
						this->LogDebugMessage("Error, No Routing", cs);
					}
				}
				else {
					routecheck = true;
				}
				if (routecheck && vait->adest == arr) { //check specified destinations like LOWI, LOWS, etc.

					if (this->checkMinMaxRFL && ((cad.GetFinalAltitude() == 0 && fpd.GetFinalAltitude() > vait->maxlvl * 100) || cad.GetFinalAltitude() > vait->maxlvl * 100)) {

						res.valid = false;
						res.tag = "MAX";
						res.color = TAG_COLOR_ORANGE;

						if (!validateOnly) {
							std::ostringstream msg;
							msg << "Flights from " << dep << " to " << arr << " via " << sid.wp << " have a maximum FL of " << vait->maxlvl;

							if (this->logMinMaxRFL) {
								this->LogMessage(msg.str(), cs);
							}
							else {
								this->LogDebugMessage(msg.str(), cs);
							}
						}

						return res;
					}
					if (this->checkMinMaxRFL && ((cad.GetFinalAltitude() == 0 && fpd.GetFinalAltitude() < vait->minlvl * 100) || (cad.GetFinalAltitude() != 0 && cad.GetFinalAltitude() < vait->minlvl * 100))) {

						res.valid = false;
						res.tag = "MIN";
						res.color = TAG_COLOR_ORANGE;

						if (!validateOnly) {
							std::ostringstream msg;
							msg << "Flights from " << dep << " to " << arr << " via " << sid.wp << " have a minimum FL of " << vait->minlvl;

							if (this->logMinMaxRFL) {
								this->LogMessage(msg.str(), cs);
							}
							else {
								this->LogDebugMessage(msg.str(), cs);
							}
						}

						return res;
					}

					//case all correct
					res.valid = true;
					res.tag = "";
					res.color = TAG_COLOR_NONE;

					return res;

				}
				else if (routecheck && vait->adest != arr && vait->adest == "") { // check for non specified destinations
					if (this->checkMinMaxRFL && ((cad.GetFinalAltitude() == 0 && fpd.GetFinalAltitude() > vait->maxlvl * 100) || cad.GetFinalAltitude() > vait->maxlvl * 100)) {

						res.valid = false;
						res.tag = "MAX";
						res.color = TAG_COLOR_ORANGE;

						if (!validateOnly) {
							std::ostringstream msg;
							msg << "Flights from " << dep << " via " << sid.wp << " have a maximum FL of " << vait->maxlvl;

							if (this->logMinMaxRFL) {
								this->LogMessage(msg.str(), cs);
							}
							else {
								this->LogDebugMessage(msg.str(), cs);
							}
						}

						break;
					}
					if (this->checkMinMaxRFL && ((cad.GetFinalAltitude() == 0 && fpd.GetFinalAltitude() < vait->minlvl * 100) || (cad.GetFinalAltitude() != 0 && cad.GetFinalAltitude() < vait->minlvl * 100))) {

						res.valid = false;
						res.tag = "MIN";
						res.color = TAG_COLOR_ORANGE;

						if (!validateOnly) {
							std::ostringstream msg;
							msg << "Flights from " << dep << " via " << sid.wp << " have a minimum FL of " << vait->minlvl;

							if (this->logMinMaxRFL) {
								this->LogMessage(msg.str(), cs);
							}
							else {
								this->LogDebugMessage(msg.str(), cs);
							}
						}

						break;
					}

					//case all correct
					res.valid = true;
					res.tag = "";
					res.color = TAG_COLOR_NONE;

					return res;

				}
				else if (this->CheckFlightPlanProcessed(fp)) {
					res.valid = false;
					res.tag = "INV";
					res.color = TAG_COLOR_ORANGE;

					continue;
				}
				else {
					res.valid = false;
					res.tag = "";
					res.color = TAG_COLOR_NONE;
					continue;
				}
			}
		}
	}
	return res;
}

bool CDelHel::CheckFlightPlanProcessed(const EuroScopePlugIn::CFlightPlan& fp)
{
	std::string cs = fp.GetCallsign();

	if (std::find(this->processed.begin(), this->processed.end(), cs) != this->processed.end()) {
		return true;
	}
	return false;
}

bool CDelHel::IsFlightPlanProcessed(const EuroScopePlugIn::CFlightPlan& fp)
{
	std::string cs = fp.GetCallsign();

	if (std::find(this->processed.begin(), this->processed.end(), cs) != this->processed.end()) {
		return true;
	}

	this->processed.push_back(cs);
	return false;
}

void CDelHel::AutoProcessFlightPlans()
{
	for (EuroScopePlugIn::CRadarTarget rt = this->RadarTargetSelectFirst(); rt.IsValid(); rt = this->RadarTargetSelectNext(rt)) {
		EuroScopePlugIn::CRadarTargetPositionData pos = rt.GetPosition();
		// Skip auto-processing if aircraft is not on the ground (currently using flightlevel threshold)
		// TODO better option for finding aircraft on ground
		if (!pos.IsValid() || pos.GetFlightLevel() > AUTO_ASSIGN_MIN_FL) {
			continue;
		}

		EuroScopePlugIn::CFlightPlan fp = rt.GetCorrelatedFlightPlan();
		// Skip auto-processing if aircraft is tracked (with exception of aircraft tracked by current controller)
		if (!fp.IsValid() || (strcmp(fp.GetTrackingControllerId(), "") != 0 && !fp.GetTrackingControllerIsMe())) {
			continue;
		}

		std::string dep = fp.GetFlightPlanData().GetOrigin();
		to_upper(dep);

		std::string arr = fp.GetFlightPlanData().GetDestination();
		to_upper(arr);

		// Skip auto-processing for aircraft without a valid flightplan (no departure/destination airport)
		if (dep == "" || arr == "") {
			continue;
		}

		auto ait = this->airports.find(dep);
		// Skip auto-processing of departures not available in the airport config
		if (ait == this->airports.end()) {
			continue;
		}
		// Skip auto-processing of airports currently not set as active in EuroScope
		if (!ait->second.active) {
			continue;
		}

		if (this->IsFlightPlanProcessed(fp)) {
			continue;
		}

		this->ProcessFlightPlan(fp, this->assignNap);
	}
}

void CDelHel::LogMessage(std::string message)
{
	this->DisplayUserMessage("Message", PLUGIN_NAME, message.c_str(), true, true, true, false, false);
}

void CDelHel::LogMessage(std::string message, std::string type)
{
	this->DisplayUserMessage(PLUGIN_NAME, type.c_str(), message.c_str(), true, true, true, this->flashOnMessage, false);
}

void CDelHel::LogDebugMessage(std::string message)
{
	if (this->debug) {
		this->LogMessage(message);
	}
}

void CDelHel::LogDebugMessage(std::string message, std::string type)
{
	if (this->debug) {
		this->LogMessage(message, type);
	}
}

void CDelHel::CheckForUpdate()
{
	try
	{
		semver::version latest{ this->latestVersion.get() };
		semver::version current{ PLUGIN_VERSION };

		if (latest > current) {
			std::ostringstream ss;
			ss << "A new version (" << latest << ") of " << PLUGIN_NAME << " is available, download it at " << PLUGIN_LATEST_DOWNLOAD_URL;

			this->LogMessage(ss.str(), "Update");
		}
	}
	catch (std::exception& e)
	{
		MessageBox(NULL, e.what(), PLUGIN_NAME, MB_OK | MB_ICONERROR);
	}

	this->latestVersion = std::future<std::string>();
}

void CDelHel::CheckLoadedPlugins()
{
	this->topSkyAvailable = false;
	this->ccamsAvailable = false;

	HMODULE hMods[1024];
	HANDLE hProcess;
	DWORD cbNeeded;
	unsigned int i;

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
	if (hProcess == NULL) {
		this->LogDebugMessage("Failed to check loaded plugins");
		return;
	}

	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
			TCHAR szModName[MAX_PATH];
			if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
				std::string moduleName = szModName;
				size_t pos = moduleName.find_last_of("\\");
				if (pos != std::string::npos) {
					moduleName = moduleName.substr(pos + 1);
				}

				if (moduleName == TOPSKY_DLL_NAME) {
					this->topSkyAvailable = true;
					this->LogDebugMessage("Found TopSky plugin", "Config");
				}
				else if (moduleName == CCAMS_DLL_NAME) {
					this->ccamsAvailable = true;
					this->LogDebugMessage("Found CCAMS plugin", "Config");
				}
			}
		}
	}

	CloseHandle(hProcess);
}

void __declspec (dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
	*ppPlugInInstance = pPlugin = new CDelHel();
}

void __declspec (dllexport) EuroScopePlugInExit(void)
{
	delete pPlugin;
}