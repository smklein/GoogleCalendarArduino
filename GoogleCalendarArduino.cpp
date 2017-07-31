/*
 * Copyright 2017 Sean Klein
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <JsonListener.h>
#include <JsonStreamingParser.h>
#include <WString.h>
#include <WiFiClientSecure.h>
#include <Time.h>

#include "GoogleCalendarArduino.h"

String RFC3339String(time_t time) {
  return String(year(time)) + "-" + \
         String(month(time)) + "-" + \
         String(day(time)) + "T" +
         String(hour(time)) + "%3A" +
         String(minute(time)) + "%3A" +
         String(second(time)) + "-07%3A00";
}

int GoogleCalendar::ListEvents(WiFiClientSecure& client, const String& accessToken) {
  String command =
    "GET https://www.googleapis.com/calendar/v3/calendars/primary/events?" \
    "orderBy=startTime" \
    "&singleEvents=true";

  Serial.println("Time: ");
  Serial.println(year());
  Serial.println(month());
  Serial.println(day());
  Serial.println(hour());

  command += "&timeMin=" + RFC3339String(now());
  command += "&timeMax=" + RFC3339String(now() + SECS_PER_DAY);
  command += "&access_token=" + accessToken;

  Serial.println(command);
  String body = "";

  GoogleCalendarListEvents listener;
  JsonStreamingParser parser;
  parser.setListener(&listener);

  if (!client.connect(GCAL_HOST, GCAL_SSL_PORT)) {
    Serial.println("Gcal failed to connect");
    return -1;
  }
  client.println(command);

  long now = millis();
  while (millis() - now < 3000) {
    while (client.available()) {
      char c = client.read();
      parser.parse(c);
    }
  }
  Serial.println("Gcal done parsing");
  return 0;
}
