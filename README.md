# Locker-System
A locker system which provide 3 alternatives for the user to lock a locker. A NuMicro MCU learning board, NU-LB-NUC140 was used in this project.

## Project Description 
1.	Register passcode by using 3x3 keypad
To register the passcode by using passcode, first the user has to enter 1 on the Home Page. Then the user was prompted to enter the 4-digit passcode. While entering the passcode, the number will show on LCD so that the user knows what they have entered. After a 4-digit passcode was entered a blue RGB LED lighted up and a beep sound was heard indicates that the passcode was register successfully. The Lockers will be locked by the 4-digit passcode. The LCD display also shows the time locked. On the same time, 4 red LEDs lighted up indicates the number of trials to enter the 4-digit passcode for unlock purpose. 

2.	Passcode security
The users were given a total of 4 trials. If the users correctly entered the registered passcode, then the Lockers will be unlocked, the LCD display will display “Correct! Unlocked!”, the RGB LED turned green and beep 2 sounds. However, when the passcode was entered wrongly for the first time, a longer beep sound was produced on the same time only 3 red LEDs will be light up. If the user entered wrongly again, same beep sound was produced on the same time 2 red LEDs left light on. Same as for the third wrongly entered passcode, only 1 red LED will be left light on. If the fourth trial was used up, then the Lockers will go into the ‘Emergency Locked’ status. 

3.	Admin RFID
Only the admin with the registered RFID card able to unlock the Lockers. On the same time, while the Lockers is in the ‘Emergency Locked’ status the RGB LED will turn to red. After the authorised RFID card was scanned, the Lockers able to be unlocked, and on the same time the LCD screen will show that it was unlocked by admin. The RGB LED will turn to green indicates that the Lockers was unlocked.  

4.	Admin Override Unlocked
Other than using RFID, Admin also have another option to unlock the lockers. Which by using  a 2 range number of 2 variable resistor. So, in order for admin to unlock the lockers, they have to adjust the value of each resistor to its range, which only admin knows the value of it. After both value are correctly adjusted, Then, admin has to enter the keypad no 9 to unlock the lockers. Once unlocked the LCD Screen will show “Override Unlocked”. The RGB LED will turn to green indicates that the Lockers was unlocked.  

## Demostration Video
https://user-images.githubusercontent.com/87352340/189520667-437becb1-b41f-42f1-a49a-38932105a60b.mp4

