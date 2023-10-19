
#include <unordered_map>
#include <string>

#include "regConfig.h"
#include "devMch.h"
#include "drvMch.h"

#include <epicsStdio.h>
#include <iocsh.h>
#include <epicsExport.h>
#include <epicsStdlib.h>
#include "string.h"

#include "yaml.h"

#define YAML_PATH "/afs/slac/g/controls/development/users/lorelli/"

static char yamlPath[4096] = YAML_PATH;

#if HAVE_EMBEDDED_YAML
#include "boardConfig.yaml.h"
#endif

/*

boards:
	- parts: ['']
	  sensors:
		- name: 'AMC 2 +12V Cur'
		  lower: [lolo, low, ...]
		  upper: [high, hihi, ...]
		  

*/

typedef struct BoardReg_ {
	epicsFloat64 lolo;
	epicsFloat64 low;
	epicsFloat64 high;
	epicsFloat64 hihi;
} BoardReg;

typedef std::unordered_map<std::string, BoardReg> BoardRegMap;

typedef struct BoardRegs_ {
	BoardRegMap regs;
} BoardRegs;

typedef std::unordered_map<std::string, BoardRegs> BoardFirmwareValuesMap;
static BoardFirmwareValuesMap boardFirmwareValues;

static void parse_upper(YAML::Node node, BoardReg& reg);
static void parse_lower(YAML::Node node, BoardReg& reg);

void regConfInit() {
	using namespace YAML;

	Node root =
	#if HAVE_EMBEDDED_YAML
		YAML::Load(s_boardConfigYaml);
	#else
		YAML::LoadFile(YAML_PATH);
	#endif
	
	if (!root)
	{
		epicsStdoutPrintf("Could not load '%s'\n", YAML_PATH);
		return;
	}

	Node boards = root["boards"];
	if (!boards || !boards.IsSequence())
	{
		epicsStdoutPrintf("'boards' key must be a sequence\n");
		return;
	}

	for (Node::iterator board = boards.begin(); board != boards.end(); ++board)
	{
		Node sensors = board->second["sensors"];
		if (!sensors)
			continue;
		
		BoardRegs regs;
		for (Node::iterator part = sensors.begin(); part != sensors.end(); ++part)
		{
			BoardReg reg;
			parse_upper(part->second["upper"], reg);
			parse_lower(part->second["lower"], reg);
			regs.regs.insert({part->second["name"].as<std::string>(), reg});
		}

		Node parts = board->second["parts"];
		if (!parts || !parts.IsSequence())
			continue;

		for (Node::iterator part = parts.begin(); part != parts.end(); ++part)
			boardFirmwareValues.insert({part->as<std::string>(), regs});
	}
}

static void parse_upper(YAML::Node node, BoardReg& reg) {
	YAML::Node::iterator it = node.begin();
	if (0 != epicsParseFloat64(it->as<std::string>().c_str(), &reg.high, NULL))
		epicsStdoutPrintf("Could not parse 'high'\n");
	++it;
	if (0 != epicsParseFloat64(it->as<std::string>().c_str(), &reg.hihi, NULL))
		epicsStdoutPrintf("Could not parse 'hihi'\n");
}

static void parse_lower(YAML::Node node, BoardReg& reg) {
	// lolo, low
	YAML::Node::iterator it = node.begin();
	if (0 != epicsParseFloat64(it->as<std::string>().c_str(), &reg.lolo, NULL))
		epicsStdoutPrintf("Could not parse 'lolo'\n");
	++it;
	if (0 != epicsParseFloat64(it->as<std::string>().c_str(), &reg.low, NULL))
		epicsStdoutPrintf("Could not parse 'low'\n");
}

void checkSensThresh(SdrFull fullSens, Sensor sens, aiRecord* pai) {
	MchRec mchRec = (MchRec)pai->dpvt;
	MchData mchDat = (MchData)mchRec->mch->udata;
	epicsFloat64 lolo = pai->lolo, hihi = pai->hihi, low = pai->low, high = pai->high;
	short fruIdx = mchDat->mchSys->fruLkup[pai->inp.value.camacio.b];

	if (fruIdx < 0)
	{
		epicsStdoutPrintf("checkSensThresh: (%s) FRU lookup failed, index=%d\n", pai->name, fruIdx);
		return;
	}

	FruRec* fru = &mchDat->mchSys->fru[fruIdx];

	if (!fru)
	{
		epicsStdoutPrintf("checkSensThresh: (%s) Invalid FRU\n", pai->name);
		return;
	}

	BoardFirmwareValuesMap::iterator it = boardFirmwareValues.find((char*)fru->board.part.data);
	if (it == boardFirmwareValues.end())
		return;
	
	BoardRegMap::iterator regit = it->second.regs.find(fullSens->str);
	if (regit == it->second.regs.end())
		return;

	#define CHECK_SENS(_s) \
		if (regit->second. _s != _s) \
			epicsStdoutPrintf("WARNING: %s: Sensor %s %s mismatches expected value. Expected %lf, but got %lf from firmware. Probably needs an update!\n", pai->name, fullSens->str, \
				#_s, regit->second. _s, _s)

	if (IPMI_SENSOR_THRESH_LC_READABLE(sens->tmask))
		CHECK_SENS(lolo);
	if (IPMI_SENSOR_THRESH_LNC_READABLE(sens->tmask))
		CHECK_SENS(low);
	if (IPMI_SENSOR_THRESH_UC_READABLE(sens->tmask))
		CHECK_SENS(hihi);
	if (IPMI_SENSOR_THRESH_UNC_READABLE(sens->tmask))
		CHECK_SENS(high);

	#undef CHECK_SENS
}

static void regConfigLoadYaml(const iocshArgBuf* buf)
{
	if (!buf[0].sval)
	{
		epicsStdoutPrintf("regConfigLoadYaml: Needs at least one parameter\n");
		return;
	}

	epicsStdoutPrintf("Loading YAML register config %s...\n", yamlPath);
	strncpy(yamlPath, buf[0].sval, sizeof(yamlPath)-1);
	yamlPath[sizeof(yamlPath)-1] = 0;

	regConfInit();
}

static void regConfigRegisterCommands()
{
	{
		static const iocshArg arg = {"file", iocshArgString};
		static const iocshArg* args[] = {&arg};
		static const iocshFuncDef func = {"regConfigLoad", 1, args};
		iocshRegister(&func, regConfigLoadYaml);
	}
}

epicsExportRegistrar(regConfigRegisterCommands);

/* vim: noet ts=4 */
