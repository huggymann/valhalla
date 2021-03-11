#include <iostream>
#include <string>
#include <vector>

#include "baldr/graphreader.h"
#include "baldr/rapidjson_utils.h"
#include "loki/worker.h"
#include "thor/worker.h"

#include "test.h"

using namespace valhalla;
using namespace valhalla::thor;
using namespace valhalla::sif;
using namespace valhalla::loki;
using namespace valhalla::baldr;
using namespace valhalla::midgard;
using namespace valhalla::tyr;

using rp = rapidjson::Pointer;

namespace {

const auto config = test::make_config("test/data/utrecht_tiles");

void check_coords(const rapidjson::Value& a, const rapidjson::Value& b) {
  EXPECT_NEAR(a.GetArray()[0].GetDouble(), b.GetArray()[0].GetDouble(), 0.00002);
  EXPECT_NEAR(a.GetArray()[1].GetDouble(), b.GetArray()[1].GetDouble(), 0.00002);
}

void try_isochrone(loki_worker_t& loki_worker,
                   thor_worker_t& thor_worker,
                   const std::string& test_request,
                   const std::string& expected_json) {
  // compute the isochrone
  Api request;
  ParseApi(test_request, Options::isochrone, request);
  loki_worker.isochrones(request);
  auto response_json = thor_worker.isochrones(request);
  loki_worker.cleanup();
  thor_worker.cleanup();

  // Parse isochrone json responses
  rapidjson::Document response, expected_response;
  response.Parse(response_json);
  expected_response.Parse(expected_json);

  // Same number of features
  auto feature_count = rp("/features").Get(expected_response)->GetArray().Size();
  ASSERT_EQ(rp("/features").Get(response)->GetArray().Size(), feature_count);

  // Check features are in the right order and look roughly the same
  for (size_t i = 0; i < feature_count; ++i) {
    // same metadata
    auto resp_prop = rp("/features/" + std::to_string(i) + "/properties").Get(response);
    auto expt_prop = rp("/features/" + std::to_string(i) + "/properties").Get(expected_response);
    EXPECT_TRUE((resp_prop && expt_prop) || (!resp_prop && !expt_prop));
    if (expt_prop) {
      ASSERT_EQ(resp_prop->GetObject(), expt_prop->GetObject());
    }

    // same geom type
    std::string resp_type =
        rp("/features/" + std::to_string(i) + "/geometry/type").Get(response)->GetString();
    std::string expt_type =
        rp("/features/" + std::to_string(i) + "/geometry/type").Get(expected_response)->GetString();
    ASSERT_EQ(resp_type, expt_type);

    // point is special
    if (expt_type == "Point") {
      check_coords(*rp("/features/" + std::to_string(i) + "/geometry/coordinates").Get(response),
                   *rp("/features/" + std::to_string(i) + "/geometry/coordinates")
                        .Get(expected_response));
    } // iteration required
    else {
      // same geom appx
      auto resp_geom = rp("/features/" + std::to_string(i) + "/geometry/coordinates" +
                          (resp_type == "Polygon" ? "/0" : ""))
                           .Get(response)
                           ->GetArray();
      auto expt_geom = rp("/features/" + std::to_string(i) + "/geometry/coordinates" +
                          (expt_type == "Polygon" ? "/0" : ""))
                           .Get(expected_response)
                           ->GetArray();
      ASSERT_EQ(resp_geom.Size(), expt_geom.Size());
      for (size_t j = 0; j < expt_geom.Size(); ++j) {
        auto rcoord = resp_geom[j].GetArray();
        auto ecoord = expt_geom[j].GetArray();
        check_coords(rcoord, ecoord);
      }
    }
  }
}

TEST(Isochrones, Basic) {
  // Test setup
  loki_worker_t loki_worker(config);
  thor_worker_t thor_worker(config);
  GraphReader reader(config.get_child("mjolnir"));

  {
    const auto expected =
        R"({"features":[{"properties":{"fillOpacity":0.33,"fill-opacity":0.33,"opacity":0.33,"fillColor":"#bf4040","fill":"#bf4040","color":"#bf4040","contour":9,"metric":"time"},"geometry":{"coordinates":[[5.040321,52.125974],[5.033321,52.118969],[5.023255,52.123937],[5.026321,52.120692],[5.031766,52.118382],[5.029321,52.113579],[5.034321,52.116532],[5.049421,52.111037],[5.049845,52.106937],[5.053207,52.103823],[5.058321,52.102771],[5.060634,52.103937],[5.060839,52.098937],[5.057204,52.098937],[5.059005,52.098621],[5.057742,52.096937],[5.06027,52.094886],[5.062321,52.096482],[5.063768,52.093937],[5.063321,52.092392],[5.058693,52.092937],[5.054985,52.088273],[5.048321,52.086876],[5.047321,52.087985],[5.045321,52.085851],[5.048774,52.08539],[5.049321,52.083135],[5.049795,52.085463],[5.051766,52.085382],[5.050907,52.083937],[5.054198,52.081937],[5.071321,52.081586],[5.071556,52.076937],[5.069321,52.075623],[5.061002,52.077256],[5.059287,52.073903],[5.062321,52.076429],[5.066321,52.075319],[5.067613,52.070937],[5.060321,52.069593],[5.054919,52.070937],[5.060329,52.078937],[5.059321,52.08115],[5.057797,52.078461],[5.054992,52.077937],[5.05566,52.074598],[5.052834,52.07445],[5.053321,52.077735],[5.050321,52.072423],[5.051237,52.075937],[5.047708,52.078324],[5.048321,52.081429],[5.043259,52.075937],[5.042771,52.072487],[5.035321,52.073258],[5.034141,52.077937],[5.029369,52.075937],[5.030321,52.07481],[5.022061,52.075197],[5.021982,52.073598],[5.025321,52.073406],[5.026952,52.071568],[5.025209,52.067937],[5.029839,52.071419],[5.033321,52.071573],[5.063321,52.066206],[5.064688,52.063304],[5.067613,52.061937],[5.065639,52.057937],[5.06776,52.057376],[5.068263,52.054879],[5.069968,52.05829],[5.073608,52.058224],[5.071012,52.057246],[5.06975,52.051508],[5.066428,52.052044],[5.066186,52.051072],[5.072978,52.046594],[5.089321,52.046308],[5.090321,52.047264],[5.093321,52.046314],[5.094321,52.048197],[5.098321,52.046251],[5.101182,52.051076],[5.104099,52.050159],[5.10757,52.051186],[5.111485,52.050101],[5.112321,52.046219],[5.113321,52.049186],[5.114893,52.046509],[5.118321,52.046164],[5.120321,52.049163],[5.124321,52.048191],[5.141321,52.050285],[5.142321,52.048204],[5.145104,52.052154],[5.155792,52.051466],[5.156139,52.054119],[5.159029,52.054937],[5.155145,52.055761],[5.155123,52.057135],[5.160004,52.058937],[5.157321,52.059628],[5.153088,52.057704],[5.151701,52.063937],[5.149182,52.063798],[5.149126,52.065937],[5.152321,52.068117],[5.156739,52.064355],[5.159112,52.064937],[5.158932,52.066548],[5.151959,52.070937],[5.161802,52.079456],[5.163074,52.091937],[5.159648,52.093264],[5.153648,52.102937],[5.161819,52.104937],[5.154033,52.104649],[5.148684,52.1133],[5.149527,52.117937],[5.146321,52.122254],[5.144821,52.119937],[5.147974,52.113284],[5.142321,52.110978],[5.143963,52.108937],[5.141321,52.106324],[5.140321,52.108765],[5.133321,52.105371],[5.134213,52.106937],[5.131321,52.107349],[5.129321,52.104532],[5.115321,52.106504],[5.111321,52.104524],[5.110531,52.108147],[5.107983,52.107599],[5.107321,52.109955],[5.103321,52.106677],[5.100321,52.107739],[5.097321,52.105768],[5.09847,52.109086],[5.094321,52.107837],[5.092321,52.110126],[5.080321,52.112602],[5.081321,52.107534],[5.084321,52.106528],[5.087321,52.107447],[5.089651,52.105607],[5.085321,52.103602],[5.082408,52.106024],[5.077321,52.106621],[5.077321,52.104335],[5.081656,52.102272],[5.082616,52.099641],[5.078321,52.10046],[5.077321,52.098424],[5.071321,52.09879],[5.070321,52.097363],[5.055535,52.110937],[5.059321,52.112586],[5.063239,52.110019],[5.057779,52.115937],[5.058321,52.117587],[5.062832,52.117937],[5.062321,52.119025],[5.057532,52.119937],[5.058321,52.118287],[5.05593,52.119546],[5.047291,52.116937],[5.049872,52.114937],[5.055683,52.115299],[5.053596,52.111662],[5.051321,52.111621],[5.038321,52.116248],[5.035835,52.118451],[5.037321,52.121382],[5.039321,52.119996],[5.041321,52.120514],[5.038507,52.122937],[5.040321,52.125974]],"type":"LineString"},"type":"Feature"}],"type":"FeatureCollection"})";
    const auto request =
        R"({"locations":[{"lat":52.078937,"lon":5.115321}],"costing":"auto","contours":[{"time":9}],"polygons":false,"denoise":0.2,"generalize":100})";
    try_isochrone(loki_worker, thor_worker, request, expected);
  }

  {
    const auto expected =
        R"({"features":[{"properties":{"fill-opacity":0.33,"fill":"#bf4040","fillColor":"#bf4040","color":"#bf4040","fillOpacity":0.33,"opacity":0.33,"contour":15,"metric":"time"},"geometry":{"coordinates":[[[5.116321,52.105645],[5.115321,52.104355],[5.114321,52.104826],[5.113321,52.104600],[5.112321,52.104914],[5.111321,52.104359],[5.110500,52.105116],[5.109321,52.105300],[5.108855,52.104937],[5.109960,52.103937],[5.108321,52.102578],[5.107913,52.104529],[5.106321,52.105415],[5.104321,52.104936],[5.101321,52.105134],[5.100321,52.104620],[5.098321,52.104963],[5.094653,52.103937],[5.094321,52.101661],[5.095783,52.101399],[5.098321,52.098531],[5.100321,52.098286],[5.101321,52.097283],[5.103321,52.097292],[5.103321,52.095801],[5.103029,52.096645],[5.102321,52.096791],[5.095321,52.096753],[5.094321,52.097731],[5.088082,52.097698],[5.087984,52.098274],[5.089505,52.098753],[5.089656,52.099602],[5.090758,52.099937],[5.089321,52.100512],[5.088321,52.100224],[5.087321,52.101708],[5.085730,52.100937],[5.085321,52.099837],[5.082552,52.098937],[5.085321,52.098570],[5.085729,52.097937],[5.085321,52.097547],[5.081321,52.097327],[5.079969,52.097937],[5.080199,52.096815],[5.081850,52.095937],[5.079321,52.095544],[5.078765,52.094937],[5.079911,52.093527],[5.081638,52.092937],[5.078916,52.092342],[5.078764,52.090937],[5.079917,52.088533],[5.081712,52.087937],[5.081321,52.087147],[5.080254,52.087004],[5.080040,52.083937],[5.081591,52.082207],[5.081661,52.079937],[5.080818,52.079440],[5.078731,52.079347],[5.077896,52.079512],[5.077321,52.080425],[5.076321,52.080581],[5.076165,52.079937],[5.077321,52.078611],[5.082321,52.078291],[5.082937,52.076553],[5.084321,52.075779],[5.084576,52.074682],[5.083953,52.074569],[5.082548,52.076164],[5.082019,52.074937],[5.084591,52.072207],[5.084614,52.071644],[5.082187,52.071803],[5.081899,52.072515],[5.079135,52.074751],[5.078965,52.075293],[5.079751,52.075937],[5.078542,52.077158],[5.077321,52.077268],[5.075321,52.075447],[5.070758,52.074937],[5.071321,52.074578],[5.074321,52.074450],[5.074954,52.073570],[5.076549,52.073165],[5.076575,52.071683],[5.075321,52.071450],[5.074808,52.070937],[5.074994,52.069610],[5.075984,52.068937],[5.075914,52.065530],[5.081321,52.065241],[5.084070,52.062686],[5.086663,52.062279],[5.087321,52.061429],[5.089321,52.061093],[5.089618,52.061937],[5.088737,52.062937],[5.089075,52.064183],[5.090574,52.064190],[5.090922,52.063538],[5.092646,52.063262],[5.093321,52.062497],[5.094321,52.064095],[5.094646,52.063262],[5.095460,52.063076],[5.095732,52.062348],[5.100727,52.058343],[5.101298,52.056914],[5.102338,52.056920],[5.102894,52.057364],[5.104321,52.056186],[5.106467,52.056083],[5.105989,52.054937],[5.105321,52.054503],[5.104285,52.054937],[5.103555,52.053703],[5.100936,52.052322],[5.100321,52.051446],[5.096519,52.051135],[5.096321,52.051628],[5.096115,52.051143],[5.097017,52.050633],[5.102321,52.050477],[5.104321,52.048435],[5.104321,52.050805],[5.103625,52.051937],[5.104321,52.052243],[5.104799,52.051415],[5.107706,52.051322],[5.108321,52.050495],[5.111753,52.050370],[5.112321,52.048989],[5.114476,52.050937],[5.113321,52.052052],[5.112887,52.051371],[5.111029,52.051645],[5.112002,52.052937],[5.112126,52.055132],[5.113823,52.055435],[5.115321,52.056653],[5.121321,52.057215],[5.123321,52.059144],[5.125321,52.059212],[5.126321,52.058229],[5.127517,52.059133],[5.128170,52.056786],[5.129321,52.056450],[5.131798,52.056414],[5.132321,52.055961],[5.134321,52.056539],[5.135001,52.055617],[5.136321,52.055443],[5.137321,52.056354],[5.139321,52.056476],[5.139912,52.058346],[5.141636,52.059621],[5.141757,52.060937],[5.143845,52.062413],[5.144321,52.064149],[5.144809,52.062425],[5.150008,52.057624],[5.151762,52.057378],[5.152321,52.055842],[5.152518,52.058134],[5.150911,52.058527],[5.148041,52.061937],[5.148321,52.062507],[5.149321,52.062329],[5.149606,52.062652],[5.148851,52.063937],[5.148973,52.065937],[5.151094,52.068164],[5.152321,52.068291],[5.153321,52.067331],[5.154649,52.067265],[5.156321,52.065605],[5.158181,52.065937],[5.156867,52.066483],[5.155649,52.068265],[5.154321,52.068494],[5.153069,52.069685],[5.153321,52.070428],[5.155321,52.070575],[5.155562,52.071178],[5.152321,52.071521],[5.151321,52.070641],[5.149109,52.072937],[5.150066,52.073937],[5.150121,52.075137],[5.151321,52.076326],[5.152970,52.074937],[5.153321,52.073831],[5.153952,52.075306],[5.155539,52.075719],[5.155433,52.078049],[5.153321,52.076455],[5.151884,52.077937],[5.153321,52.079459],[5.154562,52.079696],[5.155321,52.080736],[5.156272,52.080937],[5.156483,52.083099],[5.154026,52.082232],[5.153663,52.081595],[5.152321,52.081513],[5.151579,52.080679],[5.150321,52.080606],[5.149321,52.079668],[5.149101,52.080937],[5.150077,52.081937],[5.149321,52.082693],[5.147794,52.082464],[5.146440,52.080818],[5.145147,52.080937],[5.145214,52.082044],[5.145978,52.082281],[5.147321,52.084140],[5.149321,52.084195],[5.150321,52.083221],[5.153321,52.083297],[5.154094,52.083937],[5.153730,52.085937],[5.154676,52.087582],[5.155744,52.087937],[5.154321,52.089503],[5.150321,52.089465],[5.149641,52.086937],[5.149321,52.086665],[5.149100,52.086937],[5.149097,52.091161],[5.152321,52.091425],[5.153321,52.090781],[5.154283,52.090937],[5.153661,52.091277],[5.153564,52.093180],[5.152149,52.094109],[5.150321,52.092548],[5.149321,52.093410],[5.148321,52.092605],[5.147371,52.092937],[5.147983,52.094275],[5.149537,52.094721],[5.149870,52.095388],[5.150898,52.095514],[5.151321,52.095204],[5.151904,52.097354],[5.154273,52.097937],[5.153523,52.098139],[5.153330,52.098946],[5.152921,52.098337],[5.152321,52.098277],[5.150917,52.098533],[5.150321,52.099574],[5.147182,52.099076],[5.146694,52.098937],[5.148722,52.098338],[5.148619,52.097639],[5.145321,52.097626],[5.144804,52.097937],[5.146216,52.098937],[5.145743,52.099937],[5.146542,52.100937],[5.146321,52.101735],[5.144990,52.101268],[5.144899,52.099937],[5.144321,52.099484],[5.142837,52.100937],[5.143321,52.101648],[5.144604,52.101937],[5.142800,52.102937],[5.140321,52.102591],[5.140028,52.102230],[5.137321,52.102157],[5.136321,52.103079],[5.135882,52.102376],[5.134149,52.102109],[5.133776,52.099937],[5.134321,52.099371],[5.135594,52.099210],[5.135575,52.096937],[5.134659,52.096599],[5.134321,52.095880],[5.132492,52.096108],[5.132153,52.096769],[5.130321,52.097087],[5.129321,52.098075],[5.127811,52.098427],[5.127004,52.099620],[5.124887,52.099371],[5.124317,52.098933],[5.124359,52.102937],[5.120321,52.103258],[5.119321,52.102698],[5.116321,52.105645]]],"type":"Polygon"},"type":"Feature"}],"type":"FeatureCollection"})";
    const auto request =
        R"({"locations":[{"lat":52.078937,"lon":5.115321}],"costing":"bicycle","costing_options":{"bicycle":{"service_penalty":0}},"contours":[{"time":15}],"polygons":true,"denoise":0.2})";
    try_isochrone(loki_worker, thor_worker, request, expected);
  }

  {
    const auto expected =
        R"({"features":[{"properties":{"fill-opacity":0.33,"fill":"#bf4040","fillColor":"#bf4040","color":"#bf4040","fillOpacity":0.33,"opacity":0.33,"contour":15,"metric":"time"},"geometry":{"coordinates":[[5.116321,52.105645],[5.115321,52.104355],[5.114321,52.104826],[5.113321,52.104600],[5.112321,52.104914],[5.111321,52.104359],[5.110500,52.105116],[5.109321,52.105300],[5.108855,52.104937],[5.109960,52.103937],[5.108321,52.102578],[5.107913,52.104529],[5.106321,52.105415],[5.104321,52.104936],[5.101321,52.105134],[5.100321,52.104620],[5.098321,52.104963],[5.094653,52.103937],[5.094321,52.101661],[5.095783,52.101399],[5.098321,52.098531],[5.100321,52.098286],[5.101321,52.097283],[5.103321,52.097292],[5.103321,52.095801],[5.103029,52.096645],[5.102321,52.096791],[5.095321,52.096753],[5.094321,52.097731],[5.088082,52.097698],[5.087984,52.098274],[5.089505,52.098753],[5.089656,52.099602],[5.090758,52.099937],[5.089321,52.100512],[5.088321,52.100224],[5.087321,52.101708],[5.085730,52.100937],[5.085321,52.099837],[5.082552,52.098937],[5.085321,52.098570],[5.085729,52.097937],[5.085321,52.097547],[5.081321,52.097327],[5.079969,52.097937],[5.080199,52.096815],[5.081850,52.095937],[5.079321,52.095544],[5.078765,52.094937],[5.079911,52.093527],[5.081638,52.092937],[5.078916,52.092342],[5.078764,52.090937],[5.079917,52.088533],[5.081712,52.087937],[5.081321,52.087147],[5.080254,52.087004],[5.080040,52.083937],[5.081591,52.082207],[5.081661,52.079937],[5.080818,52.079440],[5.078731,52.079347],[5.077896,52.079512],[5.077321,52.080425],[5.076321,52.080581],[5.076165,52.079937],[5.077321,52.078611],[5.082321,52.078291],[5.082937,52.076553],[5.084321,52.075779],[5.084576,52.074682],[5.083953,52.074569],[5.082548,52.076164],[5.082019,52.074937],[5.084591,52.072207],[5.084614,52.071644],[5.082187,52.071803],[5.081899,52.072515],[5.079135,52.074751],[5.078965,52.075293],[5.079751,52.075937],[5.078542,52.077158],[5.077321,52.077268],[5.075321,52.075447],[5.070758,52.074937],[5.071321,52.074578],[5.074321,52.074450],[5.074954,52.073570],[5.076549,52.073165],[5.076575,52.071683],[5.075321,52.071450],[5.074808,52.070937],[5.074994,52.069610],[5.075984,52.068937],[5.075914,52.065530],[5.081321,52.065241],[5.084070,52.062686],[5.086663,52.062279],[5.087321,52.061429],[5.089321,52.061093],[5.089618,52.061937],[5.088737,52.062937],[5.089075,52.064183],[5.090574,52.064190],[5.090922,52.063538],[5.092646,52.063262],[5.093321,52.062497],[5.094321,52.064095],[5.094646,52.063262],[5.095460,52.063076],[5.095732,52.062348],[5.100727,52.058343],[5.101298,52.056914],[5.102338,52.056920],[5.102894,52.057364],[5.104321,52.056186],[5.106467,52.056083],[5.105989,52.054937],[5.105321,52.054503],[5.104285,52.054937],[5.103555,52.053703],[5.100936,52.052322],[5.100321,52.051446],[5.096519,52.051135],[5.096321,52.051628],[5.096115,52.051143],[5.097017,52.050633],[5.102321,52.050477],[5.104321,52.048435],[5.104321,52.050805],[5.103625,52.051937],[5.104321,52.052243],[5.104799,52.051415],[5.107706,52.051322],[5.108321,52.050495],[5.111753,52.050370],[5.112321,52.048989],[5.114476,52.050937],[5.113321,52.052052],[5.112887,52.051371],[5.111029,52.051645],[5.112002,52.052937],[5.112126,52.055132],[5.113823,52.055435],[5.115321,52.056653],[5.121321,52.057215],[5.123321,52.059144],[5.125321,52.059212],[5.126321,52.058229],[5.127517,52.059133],[5.128170,52.056786],[5.129321,52.056450],[5.131798,52.056414],[5.132321,52.055961],[5.134321,52.056539],[5.135001,52.055617],[5.136321,52.055443],[5.137321,52.056354],[5.139321,52.056476],[5.139912,52.058346],[5.141636,52.059621],[5.141757,52.060937],[5.143845,52.062413],[5.144321,52.064149],[5.144809,52.062425],[5.150008,52.057624],[5.151762,52.057378],[5.152321,52.055842],[5.152518,52.058134],[5.150911,52.058527],[5.148041,52.061937],[5.148321,52.062507],[5.149321,52.062329],[5.149606,52.062652],[5.148851,52.063937],[5.148973,52.065937],[5.151094,52.068164],[5.152321,52.068291],[5.153321,52.067331],[5.154649,52.067265],[5.156321,52.065605],[5.158181,52.065937],[5.156867,52.066483],[5.155649,52.068265],[5.154321,52.068494],[5.153069,52.069685],[5.153321,52.070428],[5.155321,52.070575],[5.155562,52.071178],[5.152321,52.071521],[5.151321,52.070641],[5.149109,52.072937],[5.150066,52.073937],[5.150121,52.075137],[5.151321,52.076326],[5.152970,52.074937],[5.153321,52.073831],[5.153952,52.075306],[5.155539,52.075719],[5.155433,52.078049],[5.153321,52.076455],[5.151884,52.077937],[5.153321,52.079459],[5.154562,52.079696],[5.155321,52.080736],[5.156272,52.080937],[5.156483,52.083099],[5.154026,52.082232],[5.153663,52.081595],[5.152321,52.081513],[5.151579,52.080679],[5.150321,52.080606],[5.149321,52.079668],[5.149101,52.080937],[5.150077,52.081937],[5.149321,52.082693],[5.147794,52.082464],[5.146440,52.080818],[5.145147,52.080937],[5.145214,52.082044],[5.145978,52.082281],[5.147321,52.084140],[5.149321,52.084195],[5.150321,52.083221],[5.153321,52.083297],[5.154094,52.083937],[5.153730,52.085937],[5.154676,52.087582],[5.155744,52.087937],[5.154321,52.089503],[5.150321,52.089465],[5.149641,52.086937],[5.149321,52.086665],[5.149100,52.086937],[5.149097,52.091161],[5.152321,52.091425],[5.153321,52.090781],[5.154283,52.090937],[5.153661,52.091277],[5.153564,52.093180],[5.152149,52.094109],[5.150321,52.092548],[5.149321,52.093410],[5.148321,52.092605],[5.147371,52.092937],[5.147983,52.094275],[5.149537,52.094721],[5.149870,52.095388],[5.150898,52.095514],[5.151321,52.095204],[5.151904,52.097354],[5.154273,52.097937],[5.153523,52.098139],[5.153330,52.098946],[5.152921,52.098337],[5.152321,52.098277],[5.150917,52.098533],[5.150321,52.099574],[5.147182,52.099076],[5.146694,52.098937],[5.148722,52.098338],[5.148619,52.097639],[5.145321,52.097626],[5.144804,52.097937],[5.146216,52.098937],[5.145743,52.099937],[5.146542,52.100937],[5.146321,52.101735],[5.144990,52.101268],[5.144899,52.099937],[5.144321,52.099484],[5.142837,52.100937],[5.143321,52.101648],[5.144604,52.101937],[5.142800,52.102937],[5.140321,52.102591],[5.140028,52.102230],[5.137321,52.102157],[5.136321,52.103079],[5.135882,52.102376],[5.134149,52.102109],[5.133776,52.099937],[5.134321,52.099371],[5.135594,52.099210],[5.135575,52.096937],[5.134659,52.096599],[5.134321,52.095880],[5.132492,52.096108],[5.132153,52.096769],[5.130321,52.097087],[5.129321,52.098075],[5.127811,52.098427],[5.127004,52.099620],[5.124887,52.099371],[5.124317,52.098933],[5.124359,52.102937],[5.120321,52.103258],[5.119321,52.102698],[5.116321,52.105645]],"type":"LineString"},"type":"Feature"},{"geometry":{"coordinates":[[5.115328,52.078940]],"type":"MultiPoint"},"properties":{"location_index":0,"type":"snapped"},"type":"Feature"},{"geometry":{"coordinates":[5.115321,52.078937],"type":"Point"},"properties":{"location_index":0,"type":"input"},"type":"Feature"}],"type":"FeatureCollection"})";
    const auto request =
        R"({"locations":[{"lat":52.078937,"lon":5.115321}],"costing":"bicycle","costing_options":{"bicycle":{"service_penalty":0}},"contours":[{"time":15}],"show_locations":true})";
    try_isochrone(loki_worker, thor_worker, request, expected);
  }
}

} // namespace

int main(int argc, char* argv[]) {
  // user wants to try it
  if (argc > 1) {
    loki_worker_t loki_worker(config);
    thor_worker_t thor_worker(config);
    GraphReader reader(config.get_child("mjolnir"));
    Api request;
    ParseApi(argv[1], Options::isochrone, request);
    loki_worker.isochrones(request);
    std::cout << thor_worker.isochrones(request) << std::endl;
    return EXIT_SUCCESS;
  }
  // Silence logs (especially long request logging)
  logging::Configure({{"type", ""}});
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
