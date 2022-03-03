%% Servo Control
% This script contains the functions required to control the five servos on
% the robotic hand

classdef servo
   properties (SetAccess = protected)
      % Array which defines the position of each of the five servos
      positions;
      
      % Serial port
      serial;
      
      % Default servo position
      default_pos = 70;
      
      % Max and min servo positions
      max = 140;
      min = 40;
   end
   methods
      % Constructor class, initializes the serial variable, and positions
      % array
      function obj = servo(ard)
         obj.serial = ard;
         obj.positions = [obj.default_pos, obj.default_pos, obj.default_pos, obj.default_pos, obj.default_pos];
      end
      
      function output = check_input(obj,input)
          % This function ensures that the servo inputs do not exceed the
          % predefined limits
          
          % If array is not length of 5 (for 5 servos), throw error!
          arr_size = size(input);
          if arr_size ~= 5
              error('Servo position array length is not 5!');
          end
          
          % Makes sure that the inputs are within the earlire defined
          % bounds
          % For loop iterates over all elements in input array
          for n = 1:5
              if input(n) > obj.max
                  input(n) = obj.max;
              elseif input(n) < obj.min
                  input(n) = obj.min;
              end
          end
          
          % Inverts the servo position for the thumb
          input(5) = 180 - input(5);
          output = input;
      end
      
      function send_pos(obj,pos)
          % This is the code for sending the servo positions over the
          % Virtual Com Port (VCP)
          ard = obj.serial;
          TXBuf = zeros(10,1);
          M = uint16(pos);
         for CurrCh = 1:5
           P = typecast(uint16(M(CurrCh)),'uint8');
           TXBuf(2*CurrCh-1)=P(2);
           TXBuf(2*CurrCh)=P(1);    
         end

         for i = 1:10
           fwrite(ard,TXBuf(i));
         end
      end
      
      function obj = change_pos(obj,pos)
          % This function recieves the desired servo positions, checks
          % them, and then sends them to the arduino over obj.serial
          
          % Ensures array size is correct, and values are within limits
          pos = check_input(obj, pos);
          
          % Sends the positions over the virtual com port (VCP) to the Arduino / other MCU
          send_pos(obj, pos);
          
          % Updates the class' servo position value
          obj.positions = pos;
      end
   end
end