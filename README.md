# faradayrf-opus
Send Opus encoded audio via FaradayRF

## This is only a prototype!
At present it encodes and decodes Opus and performs minimal
base64 encap/decapsulation.

## To test:

- Transmit: `rec -q -t .s16 -r 16000 -c 1 - | ./opusenc | ./tx.py`
- Receive: `./rx.py | ./opusdec | play -q -t .s16 -r 16000 -c 1 -`
  
