void setup() {
  Serial.begin(115200);
}

void loop() {
  // USAGE : calc_zambretti(pressure, trend, month, wind_direction, hemisphere);
  // pressure in hPA
  // Trend "Rising", or "Falling" or "Steady"
  // Month 1..12
  // Wind direction "Calm", "N", "S", "E", "W", "NE", "SE", "SW", "NW"
  // hemiSphere "Northern", "Southern"
  calc_zambretti(950, "Falling", 1, "NW", "Northern");
  calc_zambretti(950, "Steady",  1, "NW", "Northern");
  calc_zambretti(950, "Rising",  1, "NW", "Northern");
  Serial.println("-----------------");
  calc_zambretti(970, "Falling", 4, "NW", "Northern");
  calc_zambretti(970, "Steady",  4, "NW", "Northern");
  calc_zambretti(970, "Rising",  4, "NW", "Northern");
  Serial.println("-----------------");
  calc_zambretti(970, "Falling", 4, "SW", "Northern");
  calc_zambretti(970, "Steady",  4, "SW", "Northern");
  calc_zambretti(970, "Rising",  4, "SW", "Northern");
  Serial.println("-----------------");
  calc_zambretti(990, "Falling", 6, "NW", "Northern");
  calc_zambretti(990, "Steady",  6, "NW", "Northern");
  calc_zambretti(990, "Rising",  6, "NW", "Northern");
  Serial.println("-----------------");
  calc_zambretti(1010, "Falling", 6, "SW", "Northern");
  calc_zambretti(1010, "Steady",  6, "SW", "Northern");
  calc_zambretti(1010, "Rising",  6, "SW", "Northern");
  Serial.println("-----------------");
  calc_zambretti(1030, "Falling", 9, "NW", "Northern");
  calc_zambretti(1030, "Steady",  9, "NW", "Northern");
  calc_zambretti(1030, "Rising",  9, "NW", "Northern");
  Serial.println("-----------------");
  calc_zambretti(1050, "Falling", 9, "SW", "Northern");
  calc_zambretti(1050, "Steady",  9, "SW", "Northern");
  calc_zambretti(1050, "Rising",  9, "SW", "Northern");
  Serial.println("-----------------");
  calc_zambretti(1000, "Steady",  6, "Calm", "Northern");
  Serial.println("-----------------");

  delay(5000);
}

// ### Forecast Function ##################################################################################################
String wx_forecast(char code) {
  String wx_descriptor = "";
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
    default: wx_descriptor = "Unknown";                                 break;
  }
  return wx_descriptor;
}

void calc_zambretti(float zpressure, String ztrend, int zmonth, String windDirection, String hemiSphere) {
  String wx_text;
  zpressure = CorrectForWind(zpressure, windDirection, hemiSphere);
  if (zpressure > 1030) zpressure = 1030;
  // RISING
  if (ztrend == "Rising") {
    int zambretti = round(-0.1424 * zpressure + 147.7);
    //A Summer rising, improves the prospects by 1 unit over a Winter rising
    if (zmonth < 4 || zmonth > 9) zambretti = zambretti + 1; // Increasing values makes the forecast worst!
    if (zambretti < 1)  zambretti = 1;
    if (zambretti > 13) zambretti = 13;
    switch (zambretti) {
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
  if (ztrend == "Falling") {
    int zambretti = round(-0.1078 * zpressure + 114.31);
    // A Winter falling generally results in a Z value higher by 1 unit.
    if (zmonth < 4 || zmonth > 9) zambretti = zambretti + 1; // + makes the forecast worst, - better!
    if (windDirection.startsWith("N")) zambretti = zambretti - 1;
    if (windDirection.startsWith("S")) zambretti = zambretti + 1;
    if (zambretti < 1) zambretti = 1;
    if (zambretti > 9) zambretti = 9;
    switch (zambretti) {
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
  if (ztrend == "Steady") {
    int zambretti = round(0.00054749 * zpressure * zpressure - 1.212442 * zpressure + 670.08);
    if (windDirection.startsWith("S")) zambretti = zambretti + 1;
    if (zambretti < 1)  zambretti = 1;
    if (zambretti > 10) zambretti = 10;
    switch (zambretti) {
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
  Serial.println(wx_text);
}

int CorrectForWind(int zpressure, String windDirection, String hemiSphere) {
  if (hemiSphere = "Northern") {
    if (windDirection = "E")  return zpressure - 1;
    if (windDirection = "W")  return zpressure - 3;
    if (windDirection = "SE") return zpressure - 5;
    if (windDirection = "SW") return zpressure - 7;
    if (windDirection = "S")  return zpressure - 12;
    if (windDirection = "NW") return zpressure + 2;
    if (windDirection = "NE") return zpressure + 3;
    if (windDirection = "N")  return zpressure + 5;
  }
  else
  {
    if (windDirection = "W")  return zpressure - 1;
    if (windDirection = "E")  return zpressure - 3;
    if (windDirection = "NE") return zpressure - 5;
    if (windDirection = "NW") return zpressure - 7;
    if (windDirection = "N")  return zpressure - 12;
    if (windDirection = "SW") return zpressure + 2;
    if (windDirection = "SE") return zpressure + 3;
    if (windDirection = "S")  return zpressure + 5;
  }
}
