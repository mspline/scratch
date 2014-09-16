#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iostream>
#pragma warning(push)
#pragma warning(disable:4100 4127)
//#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#pragma warning(pop)

using namespace std;

namespace qi = boost::spirit::qi;

struct FeatureToRanker : std::pair<std::string, unsigned int>
{
    typedef std::pair<std::string, unsigned int> base_t;

    // The constructors will not be necessary after switching to Visual C++ November 2013 CTP
    // that supports inheriting constructors

    FeatureToRanker() {}
    FeatureToRanker(std::string featureName, unsigned int rankerIndex) : base_t(featureName, rankerIndex) {}

    first_type GetFeatureName() const { return first; }
    second_type GetRankerIndex() const { return second; }
};

BOOST_FUSION_ADAPT_STRUCT
(
    FeatureToRanker,
    (std::string, first)
    (unsigned int, second)
)

bool parse_via_scanf(const char *szMapping, vector<FeatureToRanker>& parsedMapping)
{
    parsedMapping.clear();

    while (*szMapping != 0)
    {
        char featureName[1024];
        int index;
        int charsRead;

        if (sscanf_s(szMapping, "%1023[0-9A-Za-z_]:%d%n", featureName, _countof(featureName), &index, &charsRead) != 2)
            return false;

        if (index < 0)
            return false;

        if (find_if(parsedMapping.cbegin(), parsedMapping.cend(), [&featureName](const FeatureToRanker& e){ return e.GetFeatureName().compare(featureName) == 0; }) != parsedMapping.cend())
            return false;

        parsedMapping.emplace_back(featureName, index);

        szMapping += charsRead;
        if (*szMapping == ',' && szMapping[1] != 0)
        {
            ++szMapping;
            continue;
        }

        if (*szMapping != 0)
            return false;
    }

    return true;
}

bool parse_via_qi(const char *szMapping, vector<FeatureToRanker>& parsedMapping)
{
    parsedMapping.clear();

    // The static_assert guards the reinterpret_cast below
    //static_assert(
    //    is_base_of<FeatureToRanker::base_type, FeatureToRanker>::value &&
    //    sizeof FeatureToRanker::base_type == sizeof FeatureToRanker,
    //    "FeatureToRanker must not have any non-static members except inherited ones");

    qi::parse(
        szMapping,                                            // Start of the string to parse
        szMapping + strlen(szMapping),                        // End of the string to parse
        (*qi::char_("0-9A-Za-z_") >> ':' >> qi::uint_) % ',', // Grammar:  <uint> ':' <string> { ',' <uint> ':' <string> }
        parsedMapping);
    //    *reinterpret_cast<vector<FeatureToRanker::base_t>*>(&parsedMapping));

    if (*szMapping != 0)
        // Log "Parameter is not well-formed. Expected format: <uint> ':' <string> { ',' <uint> ':' <string> }"
        return false;

    if (!parsedMapping.empty())
        for (auto p = parsedMapping.cbegin(); p != parsedMapping.cend() - 1; ++p)
            if (find_if(p + 1, parsedMapping.cend(), [&p](const FeatureToRanker& e){ return e.GetFeatureName() == p->GetFeatureName(); }) != parsedMapping.cend())
                // Log "Parameter is invalid: duplicate feature names detected."
                return false;

    return true;
}

template <typename F> chrono::milliseconds time(F&& f)
{
    chrono::system_clock::time_point start = chrono::system_clock::now();
    for (int i = 0; i < 1000000; ++i) f();
    return chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - start);
}

int main()
{
    const char *szMapping = "feature_x:1,feature_y:4,feature_z:8";
    vector<FeatureToRanker> expectedMapping = { { "feature_x", 1 }, { "feature_y", 4 }, { "feature_z", 8 } };

    const char *szBadMapping1 = "feature_x:-1,feature_y:4,feature_z:8";
    const char *szBadMapping2 = "a22a:1,4";
    const char *szBadMapping3 = "a:4,,b:3";
    const char *szBadMapping4 = "a:4,a:5";
    const char *szBadMapping5 = "a:4,";
    const char *szBadMapping6 = "a";

    vector<FeatureToRanker> parsedMapping;

    // Sanity check

    if (!parse_via_scanf(szMapping, parsedMapping) || parsedMapping != expectedMapping) return EXIT_FAILURE;
    if (!parse_via_scanf("", parsedMapping) || !parsedMapping.empty()) return EXIT_FAILURE;
    if (parse_via_scanf(szBadMapping1, parsedMapping)) return EXIT_FAILURE;
    if (parse_via_scanf(szBadMapping2, parsedMapping)) return EXIT_FAILURE;
    if (parse_via_scanf(szBadMapping3, parsedMapping)) return EXIT_FAILURE;
    if (parse_via_scanf(szBadMapping4, parsedMapping)) return EXIT_FAILURE;
    if (parse_via_scanf(szBadMapping5, parsedMapping)) return EXIT_FAILURE;
    if (parse_via_scanf(szBadMapping6, parsedMapping)) return EXIT_FAILURE;

    if (!parse_via_qi(szMapping, parsedMapping) || parsedMapping != expectedMapping) return EXIT_FAILURE;
    if (!parse_via_qi("", parsedMapping) || !parsedMapping.empty()) return EXIT_FAILURE;
    if (parse_via_qi(szBadMapping1, parsedMapping)) return EXIT_FAILURE;
    if (parse_via_qi(szBadMapping2, parsedMapping)) return EXIT_FAILURE;
    if (parse_via_qi(szBadMapping3, parsedMapping)) return EXIT_FAILURE;
    if (parse_via_qi(szBadMapping4, parsedMapping)) return EXIT_FAILURE;
    if (parse_via_qi(szBadMapping5, parsedMapping)) return EXIT_FAILURE;
    if (parse_via_qi(szBadMapping6, parsedMapping)) return EXIT_FAILURE;

    // Timing

    cout << "Parse via scanf: " << time([&](){ parse_via_scanf(szMapping, parsedMapping); }).count() << " ms" << endl;
    cout << "Parse via qi:    " << time([&](){ parse_via_qi(szMapping, parsedMapping); }).count() << " ms" << endl;
}
