
/**
 * @file
 * @author  Francisco Andrade <franciscojavier.andradechavez@iit.it>
 * @version 1.0
 *
 * @copyright (c) IIT Fondazione Istituto Italiano
 *            di Tecnologia. All rights reserved
 *
 */
#include "amedriver.h"

#include <sstream>

using namespace std;
using namespace Ethservice;
using namespace Multitorque;


struct OptionArgument
{
  //addr
  bool addrOption = false;
  std::string ipAddress = "192.168.111.254";
  std::string deviceAddress = "192.168.111.1";


} optionArgument;

/**
 * @brief This function allow the connction from pc to board
 * @return shared pointer of FtSensorNew class
 */
std::shared_ptr<rehab::FtSensorNew> connectSensor()
{

  //Device Connection
  //TODO FIX IT this static is shit
  static auto bdm2= std::make_shared<Ethservice::EthInterfaceManager>(optionArgument.ipAddress,64321);
  auto ftBoard = bdm2->addBoard(optionArgument.deviceAddress, true, 64321, true, 64321);
  auto rc = bdm2->connectAll();
  if ( !rehab::isRCOk(rc) )
  {
    exit(-1);
  }
  auto ftSensor =  std::make_shared<rehab::FtSensorNew>(bdm2.get(), ftBoard);
  bdm2->startRecvThread();
  return ftSensor;
}

/**
 * @brief Yarp constructor
 */
yarp::dev::amedriver::amedriver(): m_sensorReadings(6),
                                                                 m_status(yarp::dev::IAnalogSensor::AS_OK)
{
    yInfo("Constructor beggining.");
    // We fill the sensor readings only once in the constructor in this example
    // In reality, the buffer will be updated once a new measurement is avaible
   m_sensorReadings[0] = 0;
    m_sensorReadings[1] = 0;
    m_sensorReadings[2] = 0;

    // Set torque on x,y,z axis
    m_sensorReadings[3] = 0;
    m_sensorReadings[4] = 0;
    m_sensorReadings[5] = 0;


    // When you update the sensor readings, you also need to update the timestamp
    m_timestamp.update();
    yInfo("Constructor end");
}


yarp::dev::amedriver::~amedriver()
{
}

bool yarp::dev::amedriver::open(yarp::os::Searchable &config)
{
    yDebug("amedriver: opening");
    std::lock_guard<std::mutex> guard(m_mutex);

    string IpAddress=config.findGroup("ipAddress").tail().get(0).asString().c_str();
    vector<string> list;
    list = Ethservice::getIpAddress();
    optionArgument.ipAddress = Ethservice::searchAddressOnList(IpAddress,list);
    if (optionArgument.ipAddress== "")
    {
         yError("Constructor beggining.");
      std::cerr << "ipAddress ["  << IpAddress << "]" << " no matching" << std::endl;
      return false;
    }
    optionArgument.deviceAddress=IpAddress;
    optionArgument.addrOption = true;

    
   ft = connectSensor();
   ft->setSampleStreamPol(Multitorque::SampleStreamPolicy_NORMAL, 10);

 return true;
}

bool yarp::dev::amedriver::close()
{
    std::lock_guard<std::mutex> guard(m_mutex);
   //TODO how to close assuring socket gets closed daq.close();
    // Is this enough?
    ft->setSampleStreamPol(Multitorque::SampleStreamPolicy_OFF, 0);
    ft.reset();
    return true;
}

yarp::dev::amedriver::amedriver(const yarp::dev::amedriver& /*other*/)
{
    // Copy is disabled
    assert(false);
}

int yarp::dev::amedriver::read(yarp::sig::Vector &out)
{
    rehab::FtSensorNew::FTSensorData ftc2;
     ft->getFTData(ftc2);

     m_sensorReadings[0] =  (double) ftc2[0];
      m_sensorReadings[1] = (double) ftc2[1];
      m_sensorReadings[2] = (double) ftc2[2];

      // Set torque on x,y,z axis
      m_sensorReadings[3] = (double) ftc2[3];;
      m_sensorReadings[4] = (double) ftc2[4];;
      m_sensorReadings[5] = (double) ftc2[5];

      m_timestamp.update();
      out = m_sensorReadings;

      return m_status;
}




// are this functions required?
int yarp::dev::amedriver::getState(int /*ch*/)
{
    std::lock_guard<std::mutex> guard(m_mutex);
        yDebug("checking state");
    return m_status;
}

int yarp::dev::amedriver::getChannels()
{
    return 6;
}

int yarp::dev::amedriver::calibrateSensor()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    //read sensorSheet file? or zeroing?
    return m_status;
}

int yarp::dev::amedriver::calibrateSensor(const yarp::sig::Vector& /*value*/)
{
    std::lock_guard<std::mutex> guard(m_mutex);

    return m_status;
}

int yarp::dev::amedriver::calibrateChannel(int /*ch*/)
{
    std::lock_guard<std::mutex> guard(m_mutex);

    return m_status;
}

int yarp::dev::amedriver::calibrateChannel(int /*ch*/, double /*v*/)
{
    std::lock_guard<std::mutex> guard(m_mutex);

    return m_status;
}

yarp::os::Stamp yarp::dev::amedriver::getLastInputStamp()
{
    return m_timestamp;
}



