#include <cctype>
#include <iostream>
#include <iomanip>

#include <osmscout/Database.h>
#include <osmscout/TypeFeatures.h>
#include <osmscout/FeatureReader.h>
#include <osmscout/util/CmdLineParsing.h>

struct Arguments
{
  bool                   help=false;
  std::string            databaseDirectory;
  osmscout::GeoCoord     location;
};

int main(int argc, char* argv[])
{
  using namespace osmscout;

  CmdLineParser             argParser("Peaks", argc,argv);
  std::vector<std::string>  helpArgs{"h","help"};
  Arguments                 args;

  argParser.AddOption(osmscout::CmdLineFlag([&args](const bool& value) {
                        args.help=value;
                      }),
                      helpArgs,
                      "Return argument help",
                      true);

  argParser.AddPositional(osmscout::CmdLineStringOption([&args](const std::string& value) {
                            args.databaseDirectory=value;
                          }),
                          "DATABASE",
                          "Directory of the database to use");

  argParser.AddPositional(osmscout::CmdLineGeoCoordOption([&args](const osmscout::GeoCoord& value) {
                            args.location=value;
                          }),
                          "LOCATION",
                          "Search center");

  osmscout::CmdLineParseResult result=argParser.Parse();

  if (result.HasError()) {
    std::cerr << "ERROR: " << result.GetErrorDescription() << std::endl;
    std::cout << argParser.GetHelp() << std::endl;
    return 1;
  } else if (args.help) {
    std::cout << argParser.GetHelp() << std::endl;
    return 0;
  }

  Database db(DatabaseParameter{});
  bool res = db.Open(args.databaseDirectory);
  assert(res);

  TypeConfigRef typeConfig(db.GetTypeConfig());
  TypeInfoRef peakType = typeConfig->GetTypeInfo("natural_peak");
  assert(peakType);
  TypeInfoSet lookupTypes(std::vector<TypeInfoRef>{peakType});

  auto entries = db.LoadNodesInRadius(args.location,
                                      lookupTypes,
                                      Kilometers(100))
    .GetNodeResults();

  EleFeatureValueReader eleReader(*typeConfig);
  NameFeatureValueReader nameReader(*typeConfig);

  std::cout << "Found " << entries.size() << " peaks around " << args.location.GetDisplayText() << ":" << std::endl;
  for (const auto &e : entries) {
    const NodeRef peak = e.GetNode();
    auto nameVal = nameReader.GetValue(peak->GetFeatureValueBuffer());
    auto eleVal = eleReader.GetValue(peak->GetFeatureValueBuffer());
    if (!nameVal || !eleVal) {
      continue; // there is no name or elevation for this peak
    }
    std::cout << "  " << nameVal->GetName() << " \t(" << eleVal->GetEle() << " m a.s.l.)" << std::endl;
  }

  entries.sort([&](const NodeRegionSearchResultEntry &a, const NodeRegionSearchResultEntry &b) -> bool {
    EleFeatureValue *aVal = eleReader.GetValue(a.GetNode()->GetFeatureValueBuffer());
    EleFeatureValue *bVal = eleReader.GetValue(b.GetNode()->GetFeatureValueBuffer());
    uint32_t aEle = aVal ? aVal->GetEle() : 0;
    uint32_t bEle = bVal ? bVal->GetEle() : 0;
    return aEle > bEle;
  });

  if (entries.empty()) {
    std::cout << "No peak found" << std::endl;
    return 0;
  }
  NodeRef highestPeak = entries.front().GetNode();
  auto nameVal = nameReader.GetValue(highestPeak->GetFeatureValueBuffer());
  auto eleVal = eleReader.GetValue(highestPeak->GetFeatureValueBuffer());

  std::cout << "Highest peak around: "
            << (nameVal ? nameVal->GetName() : "No name") << " ("
            << (eleVal ? std::to_string(eleVal->GetEle()) : "?") << " m a.s.l.)"
            << std::endl;

  db.Close();
  return 0;
}
