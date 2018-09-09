// helper functions

String formatBytes(size_t bytes) {            // convert sizes in bytes to KB and MB
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
}

String ifTimeNumber10(int number) {
  String s = "";

  if (number < 10) {
    s = "0" + (String) number;
  } else {
    s = (String) number;
  }

  return s;
}

void sensorDataError(float t, float h, float p) {       // handle 'nan' data at first boot -
  String ts = (String)t;                                // some data initialization bug
  String hs = (String)h;
  String ps = (String)p;

  if (ts.equals(hs) && hs.equals(ps) && ps.equals(ts)) {  // only 1 chance to be - when "nan"
    ESP.restart();
  }
}
