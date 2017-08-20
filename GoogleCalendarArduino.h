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

class GoogleCalendarEvent {
public:
  String summary = "";
  String location = "";
};

class GoogleCalendar {
public:
  GoogleCalendar() {}

  // TODO(smklein): Break into more reasonable methods?

  // Returns GoogleCalendarEvents for the next 24 hours,
  // given an access token to a Google Calendar.
  //
  // On success, returns the number of events in |events|,
  // which is an integer between [0, eventCount].
  // Returns -1 on error.
  int ListEvents(WiFiClientSecure& client, const String& accessToken,
                 GoogleCalendarEvent* events, size_t eventCount);
private:
};

class GoogleCalendarListEvents : public JsonListener {
public:
  GoogleCalendarListEvents(GoogleCalendarEvent* dst, size_t dstSize) :
    events_(dst), eventCount_(0), eventMax_(dstSize) {}

  int eventCount() const { return eventCount_; }

private:
  virtual void whitespace(char c) override {}
  virtual void startDocument() override {}

  virtual void key(String key) override {
    switch (state_) {
    case PARSER_DEFAULT:
      if (key == "kind") {
        state_ = PARSER_KEY_KIND;
      }
      break;
    case PARSER_EVENT:
      if (key == "summary") { state_ = PARSER_EVENT_KEY_SUMMARY; }
      else if (key == "location") { state_ = PARSER_EVENT_KEY_LOCATION; }
      break;
    }
  }

  virtual void value(String value) override {
    switch (state_) {
    case PARSER_KEY_KIND:
      if (objectDepth_ == 0 && value == "calendar#event") {
        state_ = PARSER_EVENT;
        objectDepth_ = 1;
      } else if (objectDepth_) {
        // We're parsing an object, get ready for a new key...
        state_ = PARSER_EVENT;
      } else {
        // Not parsing an object, fall to default state_...
        state_ = PARSER_DEFAULT;
      }
      break;
    case PARSER_EVENT_KEY_SUMMARY:
      events_[eventCount_].summary = value;
      state_ = PARSER_EVENT;
      break;
    case PARSER_EVENT_KEY_LOCATION:
      events_[eventCount_].location = value;
      state_ = PARSER_EVENT;
      break;
    }
  }

  virtual void startObject() override {
    switch (state_) {
    case PARSER_DEFAULT:
    case PARSER_DONE:
      break;
    default:
      // Currently parsing an event
      objectDepth_++;
    }
  }

  virtual void endObject() override {
    switch (state_) {
    case PARSER_DEFAULT:
    case PARSER_DONE:
      break;
    default:
      // Currently parsing an event
      objectDepth_--;
      if (objectDepth_ == 0) {
        eventCount_++;
        if (eventCount_ == eventMax_) {
          // We've seen all the events we have room for
          state_ = PARSER_DONE;
        } else {
          // Keep looking for events
          state_ = PARSER_DEFAULT;
        }
      }
    }
  }

  virtual void endDocument() override {}
  virtual void startArray() override {}
  virtual void endArray() override {}

  enum {
    PARSER_DEFAULT,
    PARSER_KEY_KIND,           // Saw key "kind"
    PARSER_EVENT,              // Saw key "kind", value "calendar#event"
    PARSER_EVENT_KEY_SUMMARY,  // Saw key "kind", value "calendar#event", key "summary"
    PARSER_EVENT_KEY_LOCATION, // Saw key "kind", value "calendar#event", key "location"

    PARSER_DONE,
  } state_ = PARSER_DEFAULT;

  GoogleCalendarEvent* events_;
  size_t eventCount_;
  const size_t eventMax_;
  size_t objectDepth_ = 0;
};
