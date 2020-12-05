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

	this->debug = false;
	this->updateCheck = false;
	this->assignNap = false;
	this->autoProcess = false;
	this->warnRFLBelowCFL = false;
	this->logMinMaxRFL = false;

	this->LoadSettings();

	this->ReadAirportConfig();
	this->ReadRoutingConfig();

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
			msg << "Version " << PLUGIN_VERSION << " loaded. Available commands: auto, debug, nap, reload, reset, update";

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

			return true;
		}
		else if (args[1] == "minmaxrfl") {
			if (this->logMinMaxRFL) {
				this->LogMessage("No longer logging min and max RFLs for predefined routings", "Config");
			}
			else {
				this->LogMessage("Logging min and max RFLs for predefined routings", "Config");
			}

			this->logMinMaxRFL = !this->logMinMaxRFL;

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

void CDelHel::LoadSettings()
{
	const char* settings = this->GetDataFromSettings(PLUGIN_NAME);
	if (settings) {
		std::vector<std::string> splitSettings = split(settings, SETTINGS_DELIMITER);

		if (splitSettings.size() < 5) {
			this->LogMessage("Invalid saved settings found, reverting to default.");

			this->SaveSettings();

			return;
		}

		std::istringstream(splitSettings[0]) >> this->debug;
		std::istringstream(splitSettings[1]) >> this->updateCheck;
		std::istringstream(splitSettings[2]) >> this->assignNap;
		std::istringstream(splitSettings[3]) >> this->warnRFLBelowCFL;
		std::istringstream(splitSettings[4]) >> this->logMinMaxRFL;

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
		<< this->logMinMaxRFL;

	this->SaveDataToSettings(PLUGIN_NAME, "DelHel settings", ss.str().c_str());
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
			jap.value<int>("elevation", 0) // elevation
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

			for (auto& [rwy, jsi] : jrwys.items()) {
				sidinfo si{
					rwy, // rwy
					jsi.value<std::string>("dep", ""), // dep
					jsi.value<std::string>("nap", "") // nap
				};

				s.rwys.emplace(si.rwy, si);
				ap.rwys.insert(si.rwy);
			}

			ap.sids.emplace(wp, s);

			std::ostringstream rrs;
			rrs << icao << "\\/(";
			std::copy(ap.rwys.begin(), ap.rwys.end(), std::ostream_iterator<std::string>(rrs, "|"));
			rrs << ')';

			ap.rwy_regex = std::regex(rrs.str(), std::regex_constants::ECMAScript);
		}

		this->airports.emplace(icao, ap);
	}
}


validation CDelHel::ProcessFlightPlan(const EuroScopePlugIn::CFlightPlan& fp, bool nap, bool validateOnly)
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
		if (std::regex_search(*rit, REGEX_SPEED_LEVEL_GROUP)) {
			++rit;
			continue;
		}
		else if (std::regex_search(*rit, ap.rwy_regex)) {
			++rit;
			res.tag = "RWY";
			continue;
		}

		auto sit = ap.sids.find(*rit);
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

		std::string rwy = fpd.GetDepartureRwy();
		if (rwy == "") {
			this->LogMessage("Failed to process flightplan, no runway assigned", cs);

			res.valid = false;
			res.tag = "RWY";
			res.color = TAG_COLOR_RED;

			return res;
		}

		auto sit = sid.rwys.find(rwy);
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
			sssid << sidinfo.nap;
		}
		else {
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

		this->LogDebugMessage("Successfully processed flightplan", cs);

		// Add to list of processed flightplans if not added by auto-processing already
		this->IsFlightPlanProcessed(fp);
	}


	if (ap.validroutes.size() != 0) {

		flightplan fpl = flightplan(fp.GetCallsign(), fp.GetExtractedRoute(), fpd.GetRoute()); // create fp for route validation

		bool routecheck = false;
		for (auto vait = ap.validroutes.begin(); vait != ap.validroutes.end(); ++vait) {

			routecheck = false;
			auto selsidit = vait->waypts.begin();

			if (*selsidit == sid.wp) {
				if (vait->waypts.size() > 1) {
					try {

						auto wyprouit = vait->waypts.begin();

						for (auto wypfpl = fpl.route.begin(); wypfpl != fpl.route.end(); ++wypfpl) {

							if (wypfpl->name == *wyprouit) {
								routecheck = true;

								if (wyprouit == vait->waypts.end() - 1) {
									break;
								}
								else {
									++wyprouit;
								}
							}
							else {
								routecheck = false;
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

					if ((cad.GetFinalAltitude() == 0 && fpd.GetFinalAltitude() > vait->maxlvl * 100) || cad.GetFinalAltitude() > vait->maxlvl * 100) {

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
					if ((cad.GetFinalAltitude() == 0 && fpd.GetFinalAltitude() < vait->minlvl * 100) || (cad.GetFinalAltitude() != 0 && cad.GetFinalAltitude() < vait->minlvl * 100)) {

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
					if ((cad.GetFinalAltitude() == 0 && fpd.GetFinalAltitude() > vait->maxlvl * 100) || cad.GetFinalAltitude() > vait->maxlvl * 100) {

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
					if ((cad.GetFinalAltitude() == 0 && fpd.GetFinalAltitude() < vait->minlvl * 100) || (cad.GetFinalAltitude() != 0 && cad.GetFinalAltitude() < vait->minlvl * 100)) {

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

		// Skip auto-processing for aircraft without a valid flightplan (no departure/destination airport)
		if (strcmp(fp.GetFlightPlanData().GetOrigin(), "") == 0 || strcmp(fp.GetFlightPlanData().GetDestination(), "") == 0) {
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
	this->DisplayUserMessage(PLUGIN_NAME, type.c_str(), message.c_str(), true, true, true, true, false);
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

void __declspec (dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
	*ppPlugInInstance = pPlugin = new CDelHel();
}

void __declspec (dllexport) EuroScopePlugInExit(void)
{
	delete pPlugin;
}