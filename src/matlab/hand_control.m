% This below is used to close any matlab process using the COM port
% If error, port has not been left open as it can't be closed, so skip!
try
    fclose(instrfind);
catch
    % No action required
end

freeports = serialportlist("available"); % Gets list of available serial ports
try
    freeport = freeports(1); % Get first avail port (first index is 1... weird matlab!)
catch
    error("No serial devices found"); % No serial device, abort!
end

ard = serial(freeport,'BaudRate',57600); % Open serial port at 57600 baud rate

% Initializing servo controller class (in serial.m) with above serial port
motors = servo(ard);

Fs = 256; % Sampling frequency, set on the Arduino

serv_pos = 0; % Init the servo position variable

update_motor = 0.03; % In seconds, how frequently do we need to send updated motor positions
step = 0.4; % The speed of increment

time_window = 1; % Number of seconds to have on screen at once across all graphs

num_ave = 30; % Number of samples for sliding RMS window averaging

plot_size = time_window * Fs; % Calculate amount of points within time_window based on sampling freq
time = (0:plot_size-1) / Fs;

data = zeros(1, plot_size);
rms = zeros(1, plot_size);
motor = zeros(1, plot_size);

TXBuf = zeros(10,1); % Initializing the transmit (TX) buffer

packet_size = 7;
num_read = 20;  % Max 30 as 512 bytes inbuffer


%% Butterworth filters for EMG data

% Creating a bandpass filter from 3 Hz to 30 H
[bh,ah] = butter(3,3/(Fs/2),'high'); % High pass at 3 Hz
[bl,al] = butter(3,30/(Fs/2),'low'); % Low-pass at 30 Hz

%% Graphing

% Raw EMG data
subplot(3,1,1);
plotGraph1 = plot(time,data(1,:),'-',...
    'LineWidth',2,...
    'MarkerFaceColor','w',...
    'MarkerSize',2);

title('EMG','FontSize',20);
xlabel('Time, seconds','FontSize',15);
ylabel('Voltage, V','FontSize',15);

ylim([0,1000]);
xlim([0,time_window]);

% RMS EMG 
subplot(3,1,2);
plotGraph2 = plot(time,rms(1,:),'-',...
    'LineWidth',2,...
    'MarkerFaceColor','w',...
    'MarkerSize',2);

title('RMS EMG','FontSize',20);
xlabel('Time, seconds','FontSize',15);
ylabel('RMS Voltage, V','FontSize',15);

ylim([0,100]);
xlim([0,time_window]);

% Motor position comand
subplot(3,1,3);
plotGraph3 = plot(time,motor(1,:),'-',...
    'LineWidth',2,...
    'MarkerFaceColor','w',...
    'MarkerSize',2);

title('Motor command','FontSize',20);
xlabel('Time, seconds','FontSize',15);
ylabel('Position','FontSize',15);

ylim([0,180]);
xlim([0,time_window]);

drawnow
%% Reading data setup

fopen(ard);
pause(1);
iSample = 1;
wSamp = 1;
fwrite(ard,'S');
                      
%% Main loop

while ishandle(plotGraph1) 
    
    if ard.BytesAvailable >= num_read*packet_size
        
        for iRead = 1:num_read
            
            [A,count] = fread(ard,packet_size,'uint8');
            
            % Get ADC data from bytes 4 and 5 (index 5 and 6 in matlab)
            % This is because byte 4 the most significant byte, and byte 5
            % is the least significant byte of the ADC data
            data(1,iSample)=double(swapbytes(typecast(uint8(A(5:6)), 'uint16')));
            
            % Sliding window averaging for RMS voltage calculation
            if iSample > num_ave
                rms(1,iSample) = std(filtfilt(bh,ah,data(1,iSample-num_ave:iSample)));
                rms(1,iSample) = mean(rms(1,iSample-5:iSample));
            else
                rms(1,iSample) = std(filtfilt(bh,ah,data(1,[end-num_ave+iSample:end,1:iSample])));
                rms(1,iSample) = mean(rms(1,[end-5+iSample:end,1:iSample]));
            end
                  
%% Graph for M and increment the counter    

            motor(1,iSample) = serv_pos;
 
            iSample = iSample +1;
            wSamp = wSamp+1;
            if iSample > plot_size
                iSample =1;
                
            end
            
        end

%% Update the graphs

        try
            set(plotGraph1,'YData',data(1,:));
            set(plotGraph2,'YData',rms(1,:));
            set(plotGraph3,'YData',motor(1,:));
            drawnow;
        
        catch
        end
   end

%% Calculate the new servo positions from RMS voltage
  
  serv_pos = rms(end) * 3; % Multiply by three to make it fit servo range, too small or high position is handled by servo.m class
           
%% Send the motors position to Arduino

  if wSamp>plot_size*update_motor
     % Uses the servo class in servo.m and the function change_pos to control the five servos
     motors = motors.change_pos([serv_pos,serv_pos,serv_pos,serv_pos,serv_pos]);
     wSamp=1;
  end
    
end

% Closes the serial port to prevent the COM port getting stuck in use
% ... which is annoying!
fclose(ard);








