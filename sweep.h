class sweep
{
  int _pin;
  Servo _servo;
  boolean _dir; // HIGH = right, LOW = left
  int _low;
  int _high;
  int _speed;
  int _target;
  int _current;
  boolean _cont;
  
public:
  /* c'tor */
  sweep(int pin) : _pin(pin), _dir(LOW), _low(35), _high(135), _speed(3) { new_target(); }
  sweep(int pin, int low, int high) : _pin(pin), _dir(LOW), _low(low), _high(high), _speed(3) { new_target(); }
  
  /* Getters and setters */
  void speed(int s) { _speed = s; }
  int speed() { return _speed; }
  boolean finished() { return _dir ? _target <= _current : _target >= _current; }
  int low() { return _low; }
  void low(int low)
  {
    _low = low;
    if(_low < 10)
      _low = 10;
  }
  
  int high() { return _high; }
  void high(int high)
  {
    _high = high;
    if(_high > 170)
      _high = 170;
  }
  
  int current() { return _current; }
  
  /* functions */
  void attach()
  {
    _servo.attach(_pin);
    to_middle();
  }
  
  void new_target()
  {
    _target = random(min(_low, _high), max(_low, _high));
    if(_target > _current)
      _dir = HIGH;
    else
      _dir = LOW;
      
    PRINT("New target: ");
    PRINTLN(_target);
  }
  
  void tick()
  {
    PRINT("Current: ");
    PRINTLN(_current);
    PRINT("Target: ");
    PRINTLN(_target);
    
    if(_dir && _current < _target)
    {
      _current += _speed;
      _servo.write(_current);
    }
    else if(!_dir && _current > _target)
    {
      _current -= _speed;
      _servo.write(_current);
    }
    else if(_cont)
    {
      if(_target == _low)
      {
        _target = _high;
        _dir = HIGH;
      }
      else
      {
        _target = _low;
        _dir = LOW;
      }
    }
    else
      new_target();
  }
  
  void move_to(int deg)
  {
    _servo.write(deg);
    _current = deg;
  }
  
  void inc()
  {
    move_to(_current + _speed);
  }
  
  void dec()
  {
    move_to(_current - _speed);
  }
  
  void to_low()
  {
    move_to(_low);
  }
  
  void to_middle()
  {
    move_to((_high - _low) / 2);
  }
  
  void to_high()
  {
    move_to(_high);
  }
  
  void continuous(boolean enable)
  {
    _target = _low;
    _servo.write(_target);
    _cont = enable;
  }
};
