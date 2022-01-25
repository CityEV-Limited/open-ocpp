/*
MIT License

Copyright (c) 2020 Cedric Jimenez

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef CHARGEPOINTCONFIG_H
#define CHARGEPOINTCONFIG_H

#include "IChargePointConfig.h"
#include "IniFile.h"

/** @brief Section name for the parameters */
static const std::string STACK_PARAMS = "ChargePoint";

/** @brief Charge Point stack internal configuration */
class ChargePointConfig : public ocpp::config::IChargePointConfig
{
  public:
    /** @brief Constructor */
    ChargePointConfig(ocpp::helpers::IniFile& config) : m_config(config) { }

    // Paths

    /** @brief Path to the database to store persistent data */
    std::string databasePath() const override { return getString("DatabasePath"); }
    /** @brief Path to the JSON schemas to validate the messages */
    std::string jsonSchemasPath() const override { return getString("JsonSchemasPath"); }

    // Communication parameters

    /** @brief Connection URL */
    std::string connexionUrl() const override { return getString("ConnexionUrl"); };
    /** @brief Charge point identifier */
    std::string chargePointIdentifier() const override { return getString("ChargePointIdentifier"); }
    /** @brief Connection timeout */
    std::chrono::milliseconds connectionTimeout() const override { return get<std::chrono::milliseconds>("ConnectionTimeout"); }
    /** @brief Retry interval */
    std::chrono::milliseconds retryInterval() const override { return get<std::chrono::milliseconds>("RetryInterval"); }
    /** @brief Call request timeout */
    std::chrono::milliseconds callRequestTimeout() const override { return get<std::chrono::milliseconds>("CallRequestTimeout"); }
    /** @brief Cipher list to use for TLSv1.2 connections */
    std::string tlsv12CipherList() const override { return getString("Tlsv12CipherList"); }
    /** @brief Cipher list to use for TLSv1.3 connections */
    std::string tlsv13CipherList() const override { return getString("Tlsv13CipherList"); }
    /** @brief ECDH curve to use for TLS connections */
    std::string tlsEcdhCurve() const override { return getString("TlsEcdhCurve"); }
    /** @brief Allow TLS connections using self-signed certificates
     *         (Warning : enabling this feature is not recommended in production) */
    bool tlsAllowSelfSignedCertificates() const override { return getBool("TlsAllowSelfSignedCertificates"); }
    /** @brief Allow TLS connections using expired certificates
     *         (Warning : enabling this feature is not recommended in production) */
    bool tlsAllowExpiredCertificates() const override { return getBool("TlsAllowExpiredCertificates"); }
    /** @brief Accept non trusted certificates for TLS connections
     *         (Warning : enabling this feature is not recommended in production) */
    bool tlsAcceptNonTrustedCertificates() const override { return getBool("TlsAcceptNonTrustedCertificates"); }
    /** @brief Skip server name check in certificates for TLS connections
     *         (Warning : enabling this feature is not recommended in production) */
    bool tlsSkipServerNameCheck() const override { return getBool("TlsSkipServerNameCheck"); }

    // Charge point identification

    /** @brief Charge box serial number */
    std::string chargeBoxSerialNumber() const override { return getString("ChargeBoxSerialNumber"); }
    /** @brief Charge point model */
    std::string chargePointModel() const override { return getString("ChargePointModel"); }
    /** @brief Charge point serial number */
    std::string chargePointSerialNumber() const override { return getString("ChargePointSerialNumber"); }
    /** @brief Charge point vendor */
    std::string chargePointVendor() const override { return getString("ChargePointVendor"); }
    /** @brief Firmware version */
    std::string firmwareVersion() const override { return getString("FirmwareVersion"); }
    /** @brief ICCID of the moden's SIM card */
    std::string iccid() const override { return getString("Iccid"); }
    /** @brief IMSI of the moden's SIM card */
    std::string imsi() const override { return getString("Imsi"); }
    /** @brief Main electrical meter serial number */
    std::string meterSerialNumber() const override { return getString("MeterSerialNumber"); }
    /** @brief Main electrical meter type */
    std::string meterType() const override { return getString("MeterType"); }

    // Charging

    /** @brief Nominal operating voltage (needed for Watt to Amp conversions in smart charging profiles) */
    float operatingVoltage() const override { return static_cast<float>(getFloat("OperatingVoltage")); }

    // Authent

    /** @brief Maximum number of entries in the authentication cache */
    unsigned int authentCacheMaxEntriesCount() const override { return get<unsigned int>("AuthentCacheMaxEntriesCount"); }

    // Logs

    /** @brief Maximum number of entries in the log (0 = no logs in database) */
    unsigned int logMaxEntriesCount() const override { return get<unsigned int>("LogMaxEntriesCount"); }

  private:
    /** @brief Configuration file */
    ocpp::helpers::IniFile& m_config;

    /** @brief Get a boolean parameter */
    bool getBool(const std::string& param) const { return m_config.get(STACK_PARAMS, param).toBool(); }
    /** @brief Get a floating point parameter */
    double getFloat(const std::string& param) const { return m_config.get(STACK_PARAMS, param).toFloat(); }
    /** @brief Get a string parameter */
    std::string getString(const std::string& param) const { return m_config.get(STACK_PARAMS, param); }
    /** @brief Get a value which can be created from an unsigned integer */
    template <typename T>
    T get(const std::string& param) const
    {
        return T(m_config.get(STACK_PARAMS, param).toUInt());
    }
};

#endif // CHARGEPOINTCONFIG_H
