/*
 * ZAMBRETTI test software
 * This software, the ideas and concepts is Copyright (c) David Bird 2019 and beyond.
 * All rights to this software are reserved.
 * It is prohibited to redistribute or reproduce of any part or all of the software contents in any form other than the following: 
 * 1. You may print or download to a local hard disk extracts for your personal and non-commercial use only. 
 * 2. You may copy the content to individual third parties for their personal use, but only if you acknowledge the author David Bird as the source of the material. 
 * 3. You may not, except with my express written permission, distribute or commercially exploit the content. 
 * 4. You may not transmit it or store it in any other website or other form of electronic retrieval system for commercial purposes. 
 * 5. You MUST include all of this copyright and permission notice ('as annotated') and this shall be included in all copies or substantial portions of the software and where the software use is visible to an end-user.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT.
 * FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

char Z;

void setup() {
  Serial.begin(115200);
  Serial.println();
}

void loop() {
  // USAGE : calc_zambretti(pressure, trend, month, wind_direction, hemisphere);
  // pressure - in hPA
  // trend - "Rising", or "Falling" or "Steady"
  // month  - 1..12
  // wind_direction - "Calm", "N", "S", "E", "W", "NE", "SE", "SW", "NW"
  // hemiSphere - "Northern", "Southern"
  Serial.println("Test at 1000 hPa, Summer, Falling/Steady/Rising, Calm Wind, NORTHERN hemisphere");
  calc_zambretti(1000, "Falling", 6, "calm", "Northern");
  calc_zambretti(1000, "Steady",  6, "calm", "Northern");
  calc_zambretti(1000, "Rising",  6, "calm", "Northern");
  Serial.println("Test at 1000 hPa, Winter, Falling/Steady/Rising, Calm Wind, NORTHERN hemisphere");
  calc_zambretti(1000, "Falling", 1, "calm", "Northern");
  calc_zambretti(1000, "Steady",  1, "calm", "Northern");
  calc_zambretti(1000, "Rising",  1, "calm", "Northern");
  Serial.println("-----------------");
  Serial.println("Test at 1000 hPa, Summer, Falling/Steady/Rising, Calm Wind, SOUTHERN hemisphere");
  calc_zambretti(1000, "Falling", 1, "calm", "Southern");
  calc_zambretti(1000, "Steady",  1, "calm", "Southern");
  calc_zambretti(1000, "Rising",  1, "calm", "Southern");
  Serial.println("Test at 1000 hPa, Winter, Falling/Steady/Rising, Calm Wind, SOUTHERN hemisphere");
  calc_zambretti(1000, "Falling", 6, "calm", "Southern");
  calc_zambretti(1000, "Steady",  6, "calm", "Southern");
  calc_zambretti(1000, "Rising",  6, "calm", "Southern");
  Serial.println("-----------------");
  Serial.println("Test at 1000 hPa, Summer, Falling/Steady/Rising, N Wind, NORTHERN hemisphere");
  calc_zambretti(1000, "Falling", 6, "N", "Northern");
  calc_zambretti(1000, "Steady",  6, "N", "Northern");
  calc_zambretti(1000, "Rising",  6, "N", "Northern");
  Serial.println("Test at 1000 hPa, Summer, Falling/Steady/Rising, S Wind, NORTHERN hemisphere");
  calc_zambretti(1000, "Falling", 1, "S", "Northern");
  calc_zambretti(1000, "Steady",  1, "S", "Northern");
  calc_zambretti(1000, "Rising",  1, "S", "Northern");
  Serial.println("-----------------");
  Serial.println("Test at 1000 hPa, Winter, Falling/Steady/Rising, N Wind, SOUTHERN hemisphere");
  calc_zambretti(1000, "Falling", 6, "N", "Southern");
  calc_zambretti(1000, "Steady",  6, "N", "Southern");
  calc_zambretti(1000, "Rising",  6, "N", "Southern");
  Serial.println("Test at 1000 hPa, Summer, Falling/Steady/Rising, S Wind, SOUTHERN hemisphere");
  calc_zambretti(1000, "Falling", 1, "S", "Southern");
  calc_zambretti(1000, "Steady",  1, "S", "Southern");
  calc_zambretti(1000, "Rising",  1, "S", "Southern");
  Serial.println("-----------------");
  delay(50000);
}

// ### Forecast Function ##################################################################################################
String wx_forecast(char code) {
  String wx_descriptor = "";
  Z = code;
  switch (code) {
    case 'A': wx_descriptor = "Settled fine weather";                   break;
    case 'B': wx_descriptor = "Fine weather";                           break;
    case 'C': wx_descriptor = "Becoming fine";                          break;
    case 'D': wx_descriptor = "Fine, Becoming less settled";            break;
    case 'E': wx_descriptor = "Fine, Possibly showers";                 break;
    case 'F': wx_descriptor = "Fairly fine, Improving";                 break;
    case 'G': wx_descriptor = "Fairly fine, Possibly showers early";    break;
    case 'H': wx_descriptor = "Fairly fine, Showers later";             break;
    case 'I': wx_descriptor = "Showery early, Improving";               break;
    case 'J': wx_descriptor = "Changeable, Improving";                  break;
    case 'K': wx_descriptor = "Fairly Fine, Showers likely";            break;
    case 'L': wx_descriptor = "Rather Unsettled clearing later";        break;
    case 'M': wx_descriptor = "Unsettled, Probably improving";          break;
    case 'N': wx_descriptor = "Showery bright intervals";               break;
    case 'O': wx_descriptor = "Showery becoming unsettled";             break;
    case 'P': wx_descriptor = "Changeable some rain";                   break;
    case 'Q': wx_descriptor = "Unsettled, Short fine intervals";        break;
    case 'R': wx_descriptor = "Unsettled, Rain later";                  break;
    case 'S': wx_descriptor = "Unsettled, Rain at times";               break;
    case 'T': wx_descriptor = "Very Unsettled, Finer at times";         break;
    case 'U': wx_descriptor = "Rain at times, Worse later";             break;
    case 'V': wx_descriptor = "Rain at times, Becoming very unsettled"; break;
    case 'W': wx_descriptor = "Rain at Frequent Intervals";             break;
    case 'X': wx_descriptor = "Very unsettled, Rain";                   break;
    case 'Y': wx_descriptor = "Stormy, Possibly improving";             break;
    case 'Z': wx_descriptor = "Stormy, Much rain";                      break;
    default : wx_descriptor = "Unknown";                                 break;
  }
  return wx_descriptor;
}

void calc_zambretti(float zpressure, String ztrend, int zmonth, String windDirection, String hemiSphere) {
  String wx_text;
  douuble zambretti = 0;
  ztrend.toLowerCase();
  windDirection.toLowerCase();
  hemiSphere.toLowerCase();
  if (windDirection != "calm") zpressure = CorrectForWind(zpressure, windDirection, hemiSphere);
  if (zpressure > 1030) zpressure = 1030;
  // RISING
  if (ztrend == "rising") {
    zambretti = round(-0.1437 * zpressure + 148.97);
    if (hemiSphere == "northern") 
    {
      if (zmonth < 4 || zmonth > 9) zambretti = zambretti + 1;   // + 1 to adjust the forecast in Northern H. Winter, makes the weather worst!
    }
    else
    {
      if (zmonth >= 3 && zmonth <= 10) zambretti = zambretti + 1; // + 1 to adjust the forecast in Southern H. Winter, makes the weather worst!
    }
    zambretti = constrain(zambretti, 1, 13);
    switch ((int)zambretti) {
      case 1:  wx_text = wx_forecast('A'); break; // Settled Fine
      case 2:  wx_text = wx_forecast('B'); break; // Fine Weather
      case 3:  wx_text = wx_forecast('C'); break; // Becoming Fine
      case 4:  wx_text = wx_forecast('F'); break; // Fairly Fine, Improving
      case 5:  wx_text = wx_forecast('G'); break; // Fairly Fine, Possibly showers, early
      case 6:  wx_text = wx_forecast('I'); break; // Showery Early, Improving
      case 7:  wx_text = wx_forecast('J'); break; // Changeable, Improving
      case 8:  wx_text = wx_forecast('L'); break; // Rather Unsettled Clearing Later
      case 9:  wx_text = wx_forecast('M'); break; // Unsettled, Probably Improving
      case 10: wx_text = wx_forecast('Q'); break; // Unsettled, short fine Intervals
      case 11: wx_text = wx_forecast('T'); break; // Very Unsettled, Finer at times
      case 12: wx_text = wx_forecast('Y'); break; // Stormy, possibly improving
      case 13: wx_text = wx_forecast('Z'); break; // Stormy, much rain
      default: wx_text = "unknown";        break;
    }
  }
  // FALLING
  if (ztrend == "falling") {
    zambretti = round((0.0000257935 * pow(zpressure, 3) - 0.078482105 * pow(zpressure, 2) + 79.4582219457 * zpressure - 26762.7164899421));
    if (hemiSphere == "northern") 
    {
      if (zmonth < 4 || zmonth > 9) zambretti = zambretti - 1;   // + 1 to adjust the forecast in Northern H. Winter, makes the weather worst!
    }
    else
    {
      if (zmonth >= 3 && zmonth <= 10) zambretti = zambretti - 1; // + 1 to adjust the forecast in Southern H. Winter, makes the weather worst!
    }
    zambretti = constrain(zambretti, 1, 9);
    switch ((int)zambretti) {
      case 1:  wx_text = wx_forecast('A'); break; // Settled Fine
      case 2:  wx_text = wx_forecast('B'); break; // Fine Weather
      case 3:  wx_text = wx_forecast('D'); break; // Fine Becoming Less Settled
      case 4:  wx_text = wx_forecast('H'); break; // Fairly Fine Showers Later
      case 5:  wx_text = wx_forecast('O'); break; // Showery Becoming unsettled
      case 6:  wx_text = wx_forecast('R'); break; // Unsettled, Rain later
      case 7:  wx_text = wx_forecast('U'); break; // Rain at times, worse later
      case 8:  wx_text = wx_forecast('X'); break; // Very Unsettled, Rain
      case 9:  wx_text = wx_forecast('Z'); break; // Stormy, much rain
      default: wx_text = "unknown";        break;
    }
  }
  // STEADY
  if (ztrend == "steady") {
    zambretti = (double)(0.0000258964 * pow(zpressure, 3) - 0.07753778137 * pow(zpressure, 2) + 77.2287820569 * zpressure - 25582.130426005 + 0.5);
    zambretti = constrain(zambretti, 1, 10);
    switch ((int)zambretti) {
      case 1:  wx_text = wx_forecast('A'); break; // Settled Fine
      case 2:  wx_text = wx_forecast('B'); break; // Fine Weather
      case 3:  wx_text = wx_forecast('E'); break; // Fine, Possibly showers
      case 4:  wx_text = wx_forecast('K'); break; // Fairly Fine, Showers likely
      case 5:  wx_text = wx_forecast('N'); break; // Showery Bright Intervals
      case 6:  wx_text = wx_forecast('P'); break; // Changeable some rain
      case 7:  wx_text = wx_forecast('S'); break; // Unsettled, rain at times
      case 8:  wx_text = wx_forecast('W'); break; // Rain at Frequent Intervals
      case 9:  wx_text = wx_forecast('X'); break; // Very Unsettled, Rain
      case 10: wx_text = wx_forecast('Z'); break; // Stormy, much rain
      default: wx_text = "unknown";        break;
    }
  }
  Serial.println("Z code = " + String(Z) + " " + ztrend + " Month = " + String(zmonth) + " Wind Dir = " + windDirection + " ~ "+ wx_text);
}

int CorrectForWind(int zpressure, String windDirection, String hemiSphere) {
  if (hemiSphere = "northern") {
    if (windDirection == "e")  return zpressure - 1;
    if (windDirection == "w")  return zpressure - 6;
    if (windDirection == "se") return zpressure - 11;
    if (windDirection == "sw") return zpressure - 16;
    if (windDirection == "s")  return zpressure - 20;
    if (windDirection == "nw") return zpressure + 2;
    if (windDirection == "ne") return zpressure + 4;
    if (windDirection == "n")  return zpressure + 8;
  }
  else
  { // "Southern" so reverse wind directions
    if (windDirection == "e")  return zpressure + 1;
    if (windDirection == "w")  return zpressure + 2;
    if (windDirection == "se") return zpressure + 4;
    if (windDirection == "sw") return zpressure + 6;
    if (windDirection == "s")  return zpressure + 2;
    if (windDirection == "nw") return zpressure - 5;
    if (windDirection == "ne") return zpressure - 10;
    if (windDirection == "n")  return zpressure - 20;
  }
}
