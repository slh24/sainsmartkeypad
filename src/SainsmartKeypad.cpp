#include "SainsmartKeypad.h"


static int DEFAULT_KEY_PIN = 0;
static int DEFAULT_REFRESH_RATE = 10;

static int DEFAULT_MS_TO_ACTIVATE_FAST_SCROLL = 400;
static int DEFAULT_FAST_SCROLL_TRIGGER_EVERY_MS = 100;
static int DEFAULT_MAX_FAST_SCROLL_TRIGGER_EVERY_MS = 20;


SainsmartKeypad::SainsmartKeypad(int rate, int pin)
{
  _init();
  _keyPin = pin;
  _refreshRate = rate;
}

SainsmartKeypad::SainsmartKeypad(int pin){
  _init();
  _keyPin = pin;
}

SainsmartKeypad::SainsmartKeypad()
{
  _init();
}

void SainsmartKeypad::_init()
{
  _refreshRate = DEFAULT_REFRESH_RATE,
  _msToActivateFastScroll = DEFAULT_MS_TO_ACTIVATE_FAST_SCROLL;
  _maxFastScrollTriggerRate = DEFAULT_MAX_FAST_SCROLL_TRIGGER_EVERY_MS;
  _fastScrollTriggerRate = DEFAULT_FAST_SCROLL_TRIGGER_EVERY_MS;
  _keyPin = DEFAULT_KEY_PIN;
  _prevKey = NO_KEY;
  _curKey = NO_KEY;

  arv_select_ = 741;
  arv_left_ = 503;
  arv_up_ = 142;
  arv_down_ = 326;
  arv_right_ = 0;
  arv_nokey_ = 1023;
  arv_threshold_ = 15;
}

int SainsmartKeypad::getKey_fastscroll()
{
  _curKey = getKey_periodic();
  if(_curKey == SAMPLE_WAIT)
    return SAMPLE_WAIT;  // Ignore SAMPLE_WAITs
  if(_prevKey != _curKey)
  {
    _prevKey = _curKey;
    _lastFastScrollTime = 0;
    _lastKeyChangeTime = millis();
    return _curKey; // Forward once on first press
  }
  if(_curKey == NO_KEY){
    return SAMPLE_WAIT;
  }

  // _curKey is not SAMPLE_WAIT and NOT NO_KEY here.

  if(_prevKey == _curKey)
  {
    unsigned long curmillis = millis();
    unsigned long lastKeyChangeDiff = curmillis - _lastKeyChangeTime;
    if(lastKeyChangeDiff > _msToActivateFastScroll)
    {
      // Fast Scrolling activated

      unsigned long triggerevery = _fastScrollTriggerRate / (lastKeyChangeDiff / _msToActivateFastScroll);
      if(triggerevery < _maxFastScrollTriggerRate)
        triggerevery = _maxFastScrollTriggerRate;

      if(curmillis > _lastFastScrollTime + triggerevery)
      {
        // FastTrigger NOW
        _lastFastScrollTime = curmillis;
        //Serial.print(triggerevery);
        //E
        //Serial.println("Scroll");
        return _curKey;
      }
      else
        // Key is pressed but not forwarded. Wait for next
        // fast scroll trigger.
        return SAMPLE_WAIT;
    }
    else
      // Fast scroll not yet activated
      return SAMPLE_WAIT;
  }

  // Code should not reach this.
  return 99;
  ;
}

int SainsmartKeypad::getKey_waitrelease(){
  _curKey = getKey_periodic();
  if(_curKey != SAMPLE_WAIT && _curKey != _prevKey)
  {
    _prevKey = _curKey;
    return _curKey;
  }
  return SAMPLE_WAIT;
}

int SainsmartKeypad::getKey_periodic()
{
  if (millis() > _oldTime + _refreshRate)
  {
    _oldTime = millis();
    return getKey_instant();
  }
  return SAMPLE_WAIT;
}

int SainsmartKeypad::getKey_instant()
{
  int key = NO_KEY;
  _curInput = analogRead(_keyPin);

  if (_curInput > arv_select_ - arv_threshold_ && _curInput < arv_select_ + arv_threshold_ )
  {
    key = SELECT_KEY;
  }
  else if (_curInput > arv_left_ - arv_threshold_ && _curInput < arv_left_ + arv_threshold_ )
  {
    key = LEFT_KEY;
  }
  else if (_curInput > arv_up_ - arv_threshold_ && _curInput < arv_up_ + arv_threshold_ )
  {
    key = UP_KEY;
  }
  else if (_curInput > arv_down_ - arv_threshold_ && _curInput < arv_down_ + arv_threshold_ )
  {
    key = DOWN_KEY;
  }
  else if (_curInput > arv_right_ - arv_threshold_ && _curInput < arv_right_ + arv_threshold_ )
  {
    key = RIGHT_KEY;
  }

  return key;
}

void SainsmartKeypad::setAnalogReadValues(int arv_select, int arv_left, int arv_up,
                                          int arv_down, int arv_right, int arv_nokey)
{
  arv_select_ = arv_select;
  arv_left_ = arv_left;
  arv_up_ = arv_up;
  arv_down_ = arv_down;
  arv_right_ = arv_right;
  arv_nokey_ = arv_nokey;
}

void SainsmartKeypad::setRefreshRate(int rate)
{
  _refreshRate = rate;
}
void SainsmartKeypad::setMsToActivateFastScroll(int ms)
{
  _msToActivateFastScroll = ms;
}
void SainsmartKeypad::setFastScrollTriggerRate(int rate, int maxrate)
{
  _fastScrollTriggerRate = rate;
  _maxFastScrollTriggerRate = maxrate;
  if(_fastScrollTriggerRate < _maxFastScrollTriggerRate)
    _fastScrollTriggerRate = _maxFastScrollTriggerRate;
  if(_refreshRate > _fastScrollTriggerRate)
    _refreshRate = _fastScrollTriggerRate;
  if(_refreshRate > _maxFastScrollTriggerRate)
    _refreshRate = _maxFastScrollTriggerRate;
}
