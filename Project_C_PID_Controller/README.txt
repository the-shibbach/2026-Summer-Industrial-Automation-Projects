Goal: To buid a PID controller - to have a stable light reading in the LDR. This was done using a PWM red LED as the actuator, and the LDR as the sensor. 

The project is a closed-loop control system.

Error = SP - pv, and I tried to minimise this error. I tested different values of P, I, and D, and found that it was suitable to just use P and I, and skip D. 

In the images folder, the SP vs pv graphs demonstrate the benefits of each. When just using P, there was a constant offset, because if the SP = pv, then error = 0, so output = 0. This was improved upon by adding the I term. This helped keep error = 0 while having a nonzero output. However, the system often overshot. Adding a D term helped this a bit, but just having a PI controller seemed to be fine.

With (K_p = 0.3), (K_i = 0), and (K_d = 0), the system stabilised below the setpoint, showing the expected proportional-only offset.

With (K_p = 0.3), (K_i = 0.1), and (K_d = 0), the controller reached the setpoint more accurately and gave a stable response. This was the best practical tuning I found.

I also tested disturbances by covering and uncovering the LDR, then shining a phone flashlight on it. When the LDR was covered, the controller increased the LED output and brought the reading back toward the setpoint. When the flashlight was shone on the LDR, the measured value rose far above the setpoint. This was expected because the controller can reduce the LED output to zero, but it cannot remove external light.

Also note: I added a pv filter to act as "thermal lag" otherwise the LED would flicker on and off very quickly.