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

#pragma once

#include <JsonListener.h>
#include <JsonStreamingParser.h>
#include <WString.h>
#include <WiFiClientSecure.h>

#define GCAL_HOST "www.googleapis.com"
#define GCAL_SSL_PORT 443

class GoogleCalendar {
public:
  GoogleCalendar() {}

  // TODO(smklein): Break into more reasonable methods?
  int ListEvents(WiFiClientSecure& client, const String& accessToken);
private:
};


class GoogleCalendarListEvents : public JsonListener {
public:
  GoogleCalendarListEvents() {}

private:
  virtual void whitespace(char c) override {}
  virtual void startDocument() override {}

  virtual void key(String key) override {
    switch (state) {
    case PARSER_DEFAULT:
      if (key == "kind") {
        state = PARSER_KEY_KIND;
      }
      break;
    case PARSER_EVENT:
      if (key == "summary") {
        state = PARSER_EVENT_KEY_SUMMARY;
      }
      break;
    }
  }

  virtual void value(String value) override {
    switch (state) {
    case PARSER_KEY_KIND:
      if (object_depth == 0 && value == "calendar#event") {
        state = PARSER_EVENT;
        object_depth = 1;
      } else if (object_depth) {
        // We're parsing an object, get ready for a new key...
        state = PARSER_EVENT;
      } else {
        // Not parsing an object, fall to default state...
        state = PARSER_DEFAULT;
      }
      break;
    case PARSER_EVENT_KEY_SUMMARY:
      Serial.println("Value: SUMMARY");
      Serial.println(value);
      state = PARSER_EVENT;
      break;
    }
  }

  virtual void startObject() override {
    switch (state) {
    case PARSER_DEFAULT:
    case PARSER_ERROR:
      break;
    default:
      // Currently parsing an event
      object_depth++;
    }
  }

  virtual void endObject() override {
    switch (state) {
    case PARSER_DEFAULT:
    case PARSER_ERROR:
      break;
    default:
      // Currently parsing an event
      object_depth--;
      if (object_depth == 0) {
        state = PARSER_DEFAULT;
      }
    }
  }

  virtual void endDocument() override {}
  virtual void startArray() override {}
  virtual void endArray() override {}

  enum {
    PARSER_DEFAULT,
    PARSER_KEY_KIND,          // Saw key "kind"
    PARSER_EVENT,             // Saw key "kind", value "calendar#event"
    PARSER_EVENT_KEY_SUMMARY, // Saw key "kind", value "calendar#event", key "summary"

    PARSER_ERROR,
  } state = PARSER_DEFAULT;

  size_t object_depth = 0;
  String val;
};
