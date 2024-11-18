
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include <stdlib.h>  // For random number generation
#include <math.h>
#include <time.h>  // For seeding random number generation
#include <stdio.h>
#include <stdbool.h> // Ensure you include this for boolean type
#include <string>        // Include for std::string
#include <sstream>






          
float groundLevel = 26.0f; // Assuming 0.0f is the ground level
// Player position and state variables
float playerX = 100.0f;
float playerY = groundLevel + 5; // Player's Y position

float gameSpeed = 1.0f;
float speedIncrement = 0.000001f;  // Speed increment value
float maxSpeed = 15.0f;

float starRotationAngle = 0.0f;
float rotationAngleSun = 0.0f;

int isJumping = 0;  // 1 if player is in the air
int isDucking = 0;  // 1 if player is ducking
float jumpSpeed = 10.0f;  // Increased initial jump speed
float gravity = -0.7f;    // Reduced gravity to slow the fall

int lives = 5;  // Number of lives
int score = 0;  // Player's score
int gameTime = 60;  // Total game time in seconds

// Random obstacle and collectable positions
float obstacleX = 800.0f;  // Start off-screen
float obstacleY = 0.0f + groundLevel;  // Y-position of obstacles
float collectableX = 1000.0f;
float collectableY = 30.0f + groundLevel; // Y-position for collectables (jump height)
int collectibleCollected = 0;    // 1 if collectible has been collected
int collectibleSpawnCounter = 0;

int isInvincible = 0;  // Tracks invincibility status
int invincibilityTime = 0;  // Duration counter for invincibility
float scaleFactor = 1.0f;     // Global variable for scaling
bool scalingUp = true;

float timePowerUpDuration = 10.0f;  // Duration for the time freeze power-up
float timeFreezeStart = 0.0f;
bool isTimeFrozen = false;
float remainingTimePowerUp = 0.0f;
float remainingFreezeTime = 0.0f;

float powerUpX = 1500.0f;  // Start off-screen
float powerUpY = 30.0f + groundLevel;    // Y-position (same as collectables)
int powerUpType = 0;       // 1 for invincibility, 2 for timer power-up

bool isGameOver = false;
float rotationAngleP = 0.0f;

float sunAngle = 0.0f;     // Rotation angle for the sun
float zoomFactor = 1.0f;   // Zoom factor
float zoomIncrement = 0.01f; // Amount to change the zoom each frame

bool obstacleStopped = false;
bool hasCollided = false;

bool isTimeUp = false;
// Function to initialize the OpenGL window
void init() {
    glClearColor(0.0, 0.0, 1.0, 1.0);  // Set the background color to white
    glMatrixMode(GL_PROJECTION);  // Set the matrix mode to projection for 2D rendering
    glLoadIdentity();  // Reset the current matrix
    gluOrtho2D(0.0, 800.0, 0.0, 600.0);  // Define the 2D orthographic projection (X from 0 to 800, Y from 0 to 600)

    // Seed the random number generator
    srand(static_cast<unsigned int>(time(0)));
}

float applyDucking() {
    float height;
    if (isDucking) {
        height = 5.0f;  // Reduced height when ducking
        
    } else {
        height = 20.0f;  // Normal height when not ducking
    }
    return height;
}

// Function to draw the player using 4 different primitives
void drawPlayer() {
    // Set color to black for the player
    glColor3f(0.0f, 0.0f, 0.0f);
    
    // Get adjusted player height based on ducking state
    float height = applyDucking();
    
    // Draw the body (Rectangle)
    glBegin(GL_QUADS);
    glVertex2f(playerX, playerY);  // Bottom-left
    glVertex2f(playerX + 30, playerY);  // Bottom-right
    glVertex2f(playerX + 30, playerY + height);  // Top-right
    glVertex2f(playerX, playerY + height);  // Top-left
    glEnd();
    
    // Draw the head (Circle approximation using triangle fan)
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(playerX + 15, playerY + height + 15);  // Center of the head
    for (int i = 0; i <= 20; i++) {  // Approximate circle with 20 triangles
        float angle = 2.0f * 3.1415926f * i / 20;  // Calculate angle
        float dx = cos(angle) * 10.0f;  // X offset
        float dy = sin(angle) * 10.0f;  // Y offset
        glVertex2f(playerX + 15 + dx, playerY + height + 15 + dy);  // Draw point on the circle
    }
    glEnd();
    
    // Draw the ears (Two triangles)
    glBegin(GL_TRIANGLES);
    // Left ear
    glVertex2f(playerX + 10, playerY + height + 25);  // Top of left ear
    glVertex2f(playerX + 5, playerY + height + 15);  // Bottom-left
    glVertex2f(playerX + 15, playerY + height + 15);  // Bottom-right
    
    // Right ear
    glVertex2f(playerX + 20, playerY + height + 25);  // Top of right ear
    glVertex2f(playerX + 15, playerY + height + 15);  // Bottom-left
    glVertex2f(playerX + 25, playerY + height + 15);  // Bottom-right
    glEnd();
    
    // Draw the tail (Line with a curve)
    glBegin(GL_LINE_STRIP);
    glVertex2f(playerX, playerY + 10);  // Base of the tail
    glVertex2f(playerX - 10, playerY + 15);  // Midpoint of the tail
    glVertex2f(playerX - 15, playerY + 5);  // End of the tail
    glEnd();
    
    // Draw the legs (Lines)
    glBegin(GL_LINES);
    // Left leg
    glVertex2f(playerX + 5, playerY);  // Bottom-left
    glVertex2f(playerX + 5, playerY - 5);  // Left leg
    
    // Right leg
    glVertex2f(playerX + 25, playerY);  // Bottom-right
    glVertex2f(playerX + 25, playerY - 5);  // Right leg
    glEnd();
}

void drawHearts() {
    for (int i = 0; i < lives; i++) {
        glColor3f(1.0f, 0.0f, 0.0f);  // Red for hearts
        float heartX = 50.0f + i * 40.0f; // Positioning hearts

        // Draw the left arc of the heart
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(heartX, 500.0f); // Bottom point of the heart
        for (int j = 0; j <= 20; j++) {
            float angle = 2.0f * 3.1415926f * j / 20;
            float dx = cos(angle) * 10.0f; // Radius of the arc
            float dy = sin(angle) * 10.0f;
            glVertex2f(heartX - 10.0f + dx, 530.0f + dy); // Left arc
        }
        glEnd();

        // Draw the right arc of the heart
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(heartX, 500.0f); // Bottom point of the heart
        for (int j = 0; j <= 20; j++) {
            float angle = 2.0f * 3.1415926f * j / 20;
            float dx = cos(angle) * 10.0f; // Radius of the arc
            float dy = sin(angle) * 10.0f;
            glVertex2f(heartX + 10.0f + dx, 530.0f + dy); // Right arc
        }
        glEnd();

    }
}
void drawSun() {
    // Update the rotation angle for animation
    rotationAngleSun += 2.0f; // Adjust the speed of rotation as needed
    if (rotationAngleSun >= 360.0f) {
        rotationAngleSun -= 360.0f; // Keep angle within 0-360 degrees
    }

    // Save the current matrix state
    glPushMatrix();
    
    // Translate to the position of the sun
    glTranslatef(400.0f, 450.0f, 0.0f); // Center the sun at (400, 450)

    // Apply rotation around the Z-axis
    glRotatef(rotationAngleSun, 0.0f, 0.0f, 1.0f);

    // Set the color for the sun
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow color for the sun

    // Draw the sun as a circle (main body)
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.0f); // Center of the sun
    for (int i = 0; i <= 360; i++) {
        float theta = i * 3.14159f / 180.0f; // Convert degrees to radians
        float x = 20.0f * cos(theta); // Circle radius
        float y = 20.0f * sin(theta);
        glVertex2f(x, y); // Add vertex for the circle
    }
    glEnd();

    // Draw rays of the sun using triangles
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow color for rays
    for (int i = 0; i < 12; i++) {
        float theta = i * 30.0f * 3.14159f / 180.0f; // 30-degree intervals
        
        // Draw a ray as a triangle
        glBegin(GL_TRIANGLES);
        glVertex2f(0.0f, 0.0f); // Center of the sun
        glVertex2f(30.0f * cos(theta), 30.0f * sin(theta)); // End point of ray
        glVertex2f(30.0f * cos(theta + 0.1f), 30.0f * sin(theta + 0.1f)); // Next point for ray width
        glEnd();
    }

    // Restore the matrix state
    glPopMatrix();
}

// Function to draw an obstacle with 4 different shapes (rectangle, triangle, polygon, and lines)
void drawObstacle() {
    // Set color to black for obstacles
    glColor3f(0.0f, 0.0f, 0.0f);

    // 1. Draw the main rectangle part of the obstacle
    glBegin(GL_QUADS);
        glVertex2f(obstacleX, obstacleY);  // Bottom-left corner
        glVertex2f(obstacleX + 30.0f, obstacleY);  // Bottom-right corner
        glVertex2f(obstacleX + 30.0f, obstacleY + 40.0f);  // Top-right corner
        glVertex2f(obstacleX, obstacleY + 40.0f);  // Top-left corner
    glEnd();

    // 2. Draw a triangle on top of the rectangle
    glBegin(GL_TRIANGLES);
        glVertex2f(obstacleX, obstacleY + 40.0f);  // Bottom-left corner
        glVertex2f(obstacleX + 30.0f, obstacleY + 40.0f);  // Bottom-right corner
        glVertex2f(obstacleX + 15.0f, obstacleY + 60.0f);  // Top of the triangle
    glEnd();

    

    // 4. Draw lines on the sides of the obstacle for decoration
    glBegin(GL_LINES);
        glVertex2f(obstacleX, obstacleY);  // Bottom-left
        glVertex2f(obstacleX + 30.0f, obstacleY);  // Bottom-right
        
        glVertex2f(obstacleX, obstacleY + 40.0f);  // Top-left
        glVertex2f(obstacleX + 30.0f, obstacleY + 40.0f);  // Top-right
    glEnd();
}

// Function to draw collectibles (stars)
void drawCollectible() {
    // Draw the red circle
    glColor3f(1.0f, 0.0f, 0.0f);  // Set color to red
    float radius = 20.0f;  // Radius of the circle
    int num_segments = 100; // Number of segments to make the circle smooth
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(collectableX, collectableY); // Center of the circle
        for (int i = 0; i <= num_segments; i++) {
            float angle = 2.0f * 3.14159f * float(i) / float(num_segments); // Current angle
            float x = radius * cos(angle); // X coordinate
            float y = radius * sin(angle); // Y coordinate
            glVertex2f(collectableX + x, collectableY + y); // Vertex for the circle
        }
    glEnd();

    // Update the rotation angle for the star animation
    starRotationAngle += 2.0f;  // Adjust the speed of rotation as needed
    if (starRotationAngle >= 360.0f) {
        starRotationAngle -= 360.0f;  // Keep angle within 0-360 degrees
    }

    // Draw the yellow star with rotation
    glPushMatrix();  // Save the current matrix state
    glTranslatef(collectableX, collectableY, 0.0f); // Move to the center of the star
    glRotatef(starRotationAngle, 0.0f, 0.0f, 1.0f); // Rotate around the Z-axis
    glTranslatef(-collectableX, -collectableY, 0.0f); // Move back to the original position

    glColor3f(1.0f, 1.0f, 0.0f);  // Set color to yellow
    glBegin(GL_TRIANGLES);
        // Star vertices
        glVertex2f(collectableX, collectableY + 10);  // Top vertex
        glVertex2f(collectableX + 5, collectableY + 5);  // Bottom right vertex
        glVertex2f(collectableX + 10, collectableY + 5);  // Bottom right vertex
        glVertex2f(collectableX + 2.5f, collectableY - 5);  // Bottom vertex
        glVertex2f(collectableX + 5, collectableY - 10);  // Bottom left vertex
        glVertex2f(collectableX, collectableY - 2.5f);  // Left vertex
        glVertex2f(collectableX - 5, collectableY - 10);  // Bottom left vertex
        glVertex2f(collectableX - 2.5f, collectableY - 5);  // Bottom vertex
        glVertex2f(collectableX - 10, collectableY + 5);  // Bottom left vertex
        glVertex2f(collectableX - 5, collectableY + 5);  // Bottom left vertex
    glEnd();

    glPopMatrix();  // Restore the matrix state

    // Draw the black line across the circle
    glColor3f(0.0f, 0.0f, 0.0f);  // Set color to black
    glBegin(GL_LINES);
        glVertex2f(collectableX - radius, collectableY); // Start point on the left edge
        glVertex2f(collectableX + radius, collectableY); // End point on the right edge
    glEnd();
}

void drawBorders(float screenWidth, float screenHeight) {
    // Height of the borders
    float borderHeight = 10.0f;
    float additionalLayerHeight = 5.0f;  // Height of each extra layer

    // Bottom Border (Gray)
    glColor3f(0.5f, 0.5f, 0.5f);  // Set color to gray

    // Draw the bottom border
    glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);                     // Bottom-left
        glVertex2f(screenWidth, 0.0f);              // Bottom-right
        glVertex2f(screenWidth, borderHeight);      // Top-right
        glVertex2f(0.0f, borderHeight);             // Top-left
    glEnd();

    // Extra layer 1 above the bottom border (Darker Gray)
    glColor3f(0.0f, 0.0f, 0.0f);  // Set color to black

    glBegin(GL_QUADS);
        glVertex2f(0.0f, borderHeight);                            // Bottom-left
        glVertex2f(screenWidth, borderHeight);                     // Bottom-right
        glVertex2f(screenWidth, borderHeight + additionalLayerHeight); // Top-right
        glVertex2f(0.0f, borderHeight + additionalLayerHeight);    // Top-left
    glEnd();

    // Extra layer 2 above the bottom border (Dark Gray)
    glColor3f(0.3f, 0.3f, 0.3f);  // Set color to dark gray

    glBegin(GL_QUADS);
        glVertex2f(0.0f, borderHeight + additionalLayerHeight);                          // Bottom-left
        glVertex2f(screenWidth, borderHeight + additionalLayerHeight);                   // Bottom-right
        glVertex2f(screenWidth, borderHeight + 2 * additionalLayerHeight);               // Top-right
        glVertex2f(0.0f, borderHeight + 2 * additionalLayerHeight);                     // Top-left
    glEnd();

    // Extra layer 3 above the bottom border (Light Gray)
    glColor3f(0.6f, 0.6f, 0.6f);  // Set color to lighter gray

    glBegin(GL_QUADS);
        glVertex2f(0.0f, borderHeight + 2 * additionalLayerHeight);                       // Bottom-left
        glVertex2f(screenWidth, borderHeight + 2 * additionalLayerHeight);                // Bottom-right
        glVertex2f(screenWidth, borderHeight + 3 * additionalLayerHeight);                // Top-right
        glVertex2f(0.0f, borderHeight + 3 * additionalLayerHeight);                      // Top-left
    glEnd();

    // Top Border (White)
    glColor3f(1.0f, 1.0f, 1.0f);  // Set color to white

    // Draw the top border
    glBegin(GL_QUADS);
        glVertex2f(0.0f, screenHeight - borderHeight); // Bottom-left
        glVertex2f(screenWidth, screenHeight - borderHeight); // Bottom-right
        glVertex2f(screenWidth, screenHeight);              // Top-right
        glVertex2f(0.0f, screenHeight);                     // Top-left
    glEnd();

    // Extra layer 1 below the top border (Light Gray)
    glColor3f(0.8f, 0.8f, 0.8f);  // Set color to light gray

    glBegin(GL_QUADS);
        glVertex2f(0.0f, screenHeight - borderHeight - additionalLayerHeight); // Bottom-left
        glVertex2f(screenWidth, screenHeight - borderHeight - additionalLayerHeight); // Bottom-right
        glVertex2f(screenWidth, screenHeight - borderHeight);                      // Top-right
        glVertex2f(0.0f, screenHeight - borderHeight);                             // Top-left
    glEnd();

    // Extra layer 2 below the top border (Darker Gray)
    glColor3f(0.6f, 0.6f, 0.6f);  // Set color to darker gray

    glBegin(GL_QUADS);
        glVertex2f(0.0f, screenHeight - borderHeight - 2 * additionalLayerHeight); // Bottom-left
        glVertex2f(screenWidth, screenHeight - borderHeight - 2 * additionalLayerHeight); // Bottom-right
        glVertex2f(screenWidth, screenHeight - borderHeight - additionalLayerHeight);                      // Top-right
        glVertex2f(0.0f, screenHeight - borderHeight - additionalLayerHeight);                             // Top-left
    glEnd();

    // Extra layer 3 below the top border (Black)
    glColor3f(0.0f, 0.0f, 0.0f);  // Set color to black

    glBegin(GL_QUADS);
        glVertex2f(0.0f, screenHeight - borderHeight - 3 * additionalLayerHeight); // Bottom-left
        glVertex2f(screenWidth, screenHeight - borderHeight - 3 * additionalLayerHeight); // Bottom-right
        glVertex2f(screenWidth, screenHeight - borderHeight - 2 * additionalLayerHeight);                      // Top-right
        glVertex2f(0.0f, screenHeight - borderHeight - 2 * additionalLayerHeight);                             // Top-left
    glEnd();
}


void drawTimer() {
    glColor3f(0.0f, 0.0f, 0.0f);  // Set color to black
    char timerText[20];
    sprintf(timerText, "Time: %d", gameTime);  // Create timer string

    // Render timer text
    glRasterPos2f(400.0f, 540.0f);
    for (char* c = timerText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

// Function to draw the score
void drawScore() {
    char scoreText[20];
    sprintf(scoreText, "Score: %d", score);
    glColor3f(0.0f, 0.0f, 0.0f);  // Set color to black
    glRasterPos2f(700, 550);  // Position for score
    for (char *c = scoreText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);  // Draw each character
    }
}

// Function to draw power-ups
void drawPowerUp() {
    // Update the rotation angle for animation
    rotationAngleP += 2.0f;  // Adjust the speed of rotation as needed
    if (rotationAngleP >= 360.0f) {
        rotationAngleP -= 360.0f;  // Keep angle within 0-360 degrees
    }

    // Set color and scaling based on the power-up type
    if (powerUpType == 2) {
        // Green Power-Up (Freeze Time)
        glColor3f(0.0f, 1.0f, 0.0f);  // Set color to green
        scaleFactor = 1.0f;  // No scaling for green power-up
    } else if (powerUpType == 1) {
        // Red Power-Up (Invincibility)
        glColor3f(1.0f, 0.0f, 0.0f);  // Set color to red

        // Update the scaling factor for zoom in/out animation
        if (scalingUp) {
            scaleFactor += 0.01f;  // Increase the scale
            if (scaleFactor >= 1.3f) {  // Max size limit
                scalingUp = false;
            }
        } else {
            scaleFactor -= 0.01f;  // Decrease the scale
            if (scaleFactor <= 0.8f) {  // Min size limit
                scalingUp = true;
            }
        }
    }

    // Save the current matrix state
    glPushMatrix();
    
    // Translate to the center of the power-up
    glTranslatef(powerUpX + 15.0f, powerUpY + 15.0f, 0.0f);  // Center the power-up
    
    // Apply scaling (only for red power-up)
    glScalef(scaleFactor, scaleFactor, 1.0f);

    // Apply rotation only for the green power-up
    if (powerUpType == 2) {
        glRotatef(rotationAngleP, 0.0f, 0.0f, 1.0f);  // Rotate around the Z-axis
    }

    // Translate back to the original position
    glTranslatef(-(powerUpX + 15.0f), -(powerUpY + 15.0f), 0.0f);
    
    // Draw the power-up shape
    // Draw the square
    glBegin(GL_QUADS);
    glVertex2f(powerUpX, powerUpY);               // Bottom-left
    glVertex2f(powerUpX + 30.0f, powerUpY);      // Bottom-right
    glVertex2f(powerUpX + 30.0f, powerUpY + 30.0f); // Top-right
    glVertex2f(powerUpX, powerUpY + 30.0f);      // Top-left
    glEnd();
    
    // Draw triangle on the left side
    glBegin(GL_TRIANGLES);
    glVertex2f(powerUpX - 10.0f, powerUpY + 15.0f);  // Left peak
    glVertex2f(powerUpX, powerUpY);                   // Bottom-left of square
    glVertex2f(powerUpX, powerUpY + 30.0f);           // Top-left of square
    glEnd();
    
    // Draw triangle on the right side
    glBegin(GL_TRIANGLES);
    glVertex2f(powerUpX + 40.0f, powerUpY + 15.0f);  // Right peak
    glVertex2f(powerUpX + 30.0f, powerUpY);          // Bottom-right of square
    glVertex2f(powerUpX + 30.0f, powerUpY + 30.0f);  // Top-right of square
    glEnd();
    
    // Draw triangle on the top
    glBegin(GL_TRIANGLES);
    glVertex2f(powerUpX + 15.0f, powerUpY + 40.0f);  // Top peak
    glVertex2f(powerUpX, powerUpY + 30.0f);           // Top-left of square
    glVertex2f(powerUpX + 30.0f, powerUpY + 30.0f);   // Top-right of square
    glEnd();
    
    // Restore the matrix state
    glPopMatrix();
}


void timeUp() {
    // Clear the window and set up the time up screen
    glClear(GL_COLOR_BUFFER_BIT);  // Clear the screen
    glLoadIdentity();  // Reset the coordinate system

    // Set the background color to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Set the color to white for the text
    glColor3f(1.0f, 1.0f, 1.0f);

    // Display "TIME'S UP!" text in the center
    glRasterPos2f(-0.2f, 0.2f);  // Set the position for the text
    const char* timeUpText = "TIME'S UP!";
    for (const char* c = timeUpText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);  // Render each character
    }

    // Display the player's score below the "TIME'S UP!" text
    glRasterPos2f(-0.2f, 0.0f);  // Set the position for the score
    char scoreText[30];
    sprintf(scoreText, "Your Score: %d", score);
    for (const char* c = scoreText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);  // Render each character
    }

    // Flush the buffers to ensure all rendering is displayed
    glFlush();
    glutSwapBuffers();  // Swap the buffers if you're using double buffering

    // Optionally, stop the game loop or enter a wait state here
    // gameRunning = false;  // Assuming you have a game state variable
}


// Function to update the timer every second
void updateTimer(int value) {
    // Check if time is frozen
    if (isTimeFrozen) {
        // Decrease freeze time duration
        remainingFreezeTime--;

        // If freeze time is over, unfreeze the time
        if (remainingFreezeTime <= 0) {
            isTimeFrozen = false;
        }
    }
    else {
        // Only update the gameTime if time is not frozen
        gameTime--;
    }
    
    if (gameTime < 0) {
                timeUp();  // Call timeUp function
                isTimeUp = true;
            }


    // Continue calling this function every second
    glutTimerFunc(1000, updateTimer, 0);
}

// Function to activate the time freeze power-up
void activateTimeFreeze() {
    isTimeFrozen = true;  // Activate the time freeze
    remainingFreezeTime = timePowerUpDuration;  // Set the freeze duration to 5 seconds
}

// Function to update obstacle positions
void updateObstacle() {
    if (!obstacleStopped) {
        obstacleX -= 5.0f * gameSpeed;  // Adjust speed with gameSpeed
        if (obstacleX < -30.0f) {  // If obstacle goes off-screen
            obstacleX = 800.0f;  // Reset to the right side
            if(rand() % 2 == 1)
                obstacleY = groundLevel;
            else
                obstacleY = 47.0f + groundLevel;
        }
    }
}

// Function to update collectible positions
void updateCollectible() {
    collectableX -= 5.0f * gameSpeed;  // Adjust speed with gameSpeed
    if (collectableX < -30.0f) {  // If collectible goes off-screen
        collectableX = 800.0f;  // Reset to the right side
        collectableY = (30.0f + rand() % 30) + groundLevel;  // Randomize Y position
        collectibleCollected = 0;  // Reset collection status
    }
}


// Function to update power-up positions
void updatePowerUp() {
    if (powerUpType != 0) {
        powerUpX -= 5.0f * gameSpeed;  // Adjust speed with gameSpeed
        if (powerUpX < -30.0f) {
            powerUpType = 0;  // Reset power-up if off-screen
        }
    } else {
        // Randomly spawn a power-up
        if (rand() % 200 < 1) {  // 0.5% chance to spawn a power-up
            powerUpType = rand() % 2 + 1;  // Choose between invincibility or time power-up
            powerUpX = 800.0f;  // Reset to the right side
            powerUpY = (rand() % 2 == 1) ? 30.0f + groundLevel : 60.0f + groundLevel;  // Randomize Y position
        }
    }
}


void updateSpeed(float deltaTime) {
    if (gameSpeed < maxSpeed) {
        gameSpeed += speedIncrement * deltaTime;  // Gradually increase speed
    }
}


void GameOver() {
    // Clear the window and set up the game over screen
    glClear(GL_COLOR_BUFFER_BIT);  // Clear the screen
    glLoadIdentity();  // Reset the coordinate system

    // Set the background color to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Set the color to white for the text
    glColor3f(1.0f, 1.0f, 1.0f);

    // Display "GAME OVER" text in the center
    glRasterPos2f(-0.2f, 0.2f);  // Set the position for the text
    const char* gameOverText = "GAME OVER";
    for (const char* c = gameOverText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);  // Render each character
    }

    // Display the player's score below the "GAME OVER" text
    glRasterPos2f(-0.2f, 0.0f);  // Set the position for the score
    char scoreText[30];
    sprintf(scoreText, "Your Score: %d", score);
    for (const char* c = scoreText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);  // Render each character
    }

    // Flush the buffers to ensure all rendering is displayed
    glFlush();
    glutSwapBuffers();  // Swap the buffers if you're using double buffering
}


// Function to handle player jumping
void jumpPlayer() {
    if (isJumping) {
        // Move the player up by the current jump speed
        playerY += jumpSpeed;
        obstacleStopped = false;
        hasCollided = false;
        // Apply gravity to the jump speed, making the player fall faster as they ascend
        jumpSpeed += gravity;
        
        // Check if the player has hit the ground
        if (playerY <= groundLevel) {
            playerY = groundLevel + 5;  // Reset to ground level
            isJumping = 0;  // Stop jumping
            jumpSpeed = 10.0f;  // Reset jump speed for the next jump
        }
    }
}
// Function to handle keyboard input for special keys (up, down)
void handleSpecialKeyPress(int key, int x, int y) {
    if (key == GLUT_KEY_UP && playerY == groundLevel + 5) {  // Jump only when on the ground
        isJumping = 1;  // Set jumping state
        jumpSpeed = 10.0f;  // Set initial jump speed
    }
    if (key == GLUT_KEY_DOWN) {
        isDucking = 1;  // Start ducking
    }
}

// Function to handle special key release (up, down)
void handleSpecialKeyRelease(int key, int x, int y) {
    if (key == GLUT_KEY_DOWN) {
        isDucking = 0;  // Stop ducking
    }
}

// Function to check collisions with obstacles
void checkCollisions() {
    // Ground obstacle collision (obstacleY <= 26.0f)
    if (obstacleY <= 26.0f) {
        // Check for collision with the ground obstacle
        if (!isJumping && playerX + 30 > obstacleX && playerX < obstacleX + 30 && playerY < obstacleY + 40) {
            // Player has collided with the ground obstacle
            if (!isInvincible && !hasCollided) {
                lives--;  // Decrease lives once
                isInvincible = true;  // Activate invincibility
                invincibilityTime = 60;  // 1 second of invincibility (60 frames)
                obstacleStopped = false;  // Stop obstacle movement
                hasCollided = true;  // Mark collision occurred
                
                
                // Handle game over condition if lives are exhausted
                if (lives <= 0) {
                    printf("Game Over! Your score: %d\n", score);
                    GameOver();
                }
            }
        } else if (playerX > obstacleX + 30) {
            // Player has passed the ground obstacle, reset flags
            obstacleStopped = false;
            hasCollided = false;
        }
    }

    // Air obstacle collision (obstacleY > 26.0f)
    if (obstacleY > 26.0f) {
        // Check for collision with the air obstacle
        if (!isDucking && playerX + 30 > obstacleX && playerX < obstacleX + 30 && playerY < obstacleY + 40) {
            // Player has collided with the air obstacle
            if (!isInvincible && !hasCollided) {
                lives--;  // Decrease lives once
                isInvincible = true;  // Activate invincibility
                invincibilityTime = 60;  // 1 second of invincibility (60 frames)
                obstacleStopped = true;  // Stop obstacle movement
                hasCollided = true;  // Mark collision occurred
                
                // Handle game over condition if lives are exhausted
                if (lives <= 0) {
                    printf("Game Over! Your score: %d\n", score);
                    GameOver();
                }
            }
        } else if (isDucking && hasCollided) {
            // If the player ducks after colliding, reset collision flags and let them pass
            obstacleStopped = false;
            hasCollided = false;
        } else if (playerX > obstacleX + 30) {
            // Player has passed the air obstacle, reset flags
            obstacleStopped = false;
            hasCollided = false;
        }
    }
}

// Function to check collisions with collectibles
void checkCollectibles() {
    if (playerX + 30 > collectableX && playerX < collectableX + 30 && playerY < collectableY + 30) {
        collectibleCollected += 1;  // Collect the collectible
        score += 100;  // Increase score
        collectableX = 800.0f;  // Reset collectible
    }
}

// Function to check collisions with power-ups
void checkPowerUpCollision() {
    // Ensure power-up is active and its coordinates are set
    if (powerUpType != 0) {
        // Check for collision with power-up
        if (playerX + 30 > powerUpX && playerX < powerUpX + 30 && playerY < powerUpY + 30 && playerY + 30 > powerUpY) {
            // Activate the corresponding power-up
            if (powerUpType == 1) {
                isInvincible = true;  // Activate invincibility
                invincibilityTime = 600;  // Duration in frames (assuming 60 FPS, 10 seconds = 600 frames)
            } else if (powerUpType == 2 && !isTimeFrozen) {
                isTimeFrozen = true;  // Activate time freeze
                remainingTimePowerUp = timePowerUpDuration;  // Start the countdown for power-up
            }


            // Reset power-up after collection
            powerUpType = 0;  // Reset power-up type
            powerUpX = 800.0f;  // Reset power-up position
            powerUpY = rand() % 500; // Randomize Y position if needed
        }
    }
}



void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT);  // Clear the screen

    // Check for game over condition and render accordingly
    if (lives <= 0) {
        GameOver();  // Ensure GameOver function handles the score and cleanup
    } else if(isTimeUp){
        timeUp();
    } else {
        // Draw all game elements
        drawPlayer();
        drawObstacle();
        drawCollectible();
        drawHearts();
        drawScore();
        drawTimer();
        drawPowerUp();
        drawBorders(800, 600);
        drawSun();

        // Swap buffers to display the frame
        glutSwapBuffers();
    }
}

// Function to update all game elements (called every frame)
void updateGame(int value) {
    // Only update if the game is not over and time is not up
    if (!isGameOver && !isTimeUp) {
        updateSpeed(gameTime);
        // Update positions of obstacles, collectibles, and power-ups
        updateObstacle();
        updateCollectible();
        updatePowerUp();

        // Handle player movement and jumping
        jumpPlayer();

        // Check for collisions with obstacles, collectibles, and power-ups
        checkCollisions();
        checkCollectibles();
        checkPowerUpCollision();

        // Handle invincibility timer
        if (isInvincible) {
            invincibilityTime--;
            if (invincibilityTime <= 0) {
                isInvincible = false;  // Deactivate invincibility
            }
        }

        // Check if the time frozen power-up has expired
        if (isTimeFrozen && remainingTimePowerUp <= 0) {
            isTimeFrozen = false;
        }

        // Check if lives are zero and set game over state
        if (lives <= 0) {
            isGameOver = true;
        }

        // Redraw the scene
        glutPostRedisplay();
    }

    // Call update function again after 16 milliseconds (roughly 60 frames per second)
    glutTimerFunc(16, updateGame, 0);
}

// Main function to set up the OpenGL environment
int main(int argc, char** argv) {
    glutInit(&argc, argv);  // Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);  // Set display mode to double buffering and RGB
    glutInitWindowSize(800, 600);  // Set the window size
    glutCreateWindow("2D Infinite Runner Game");  // Create window with title

    init();  // Initialize the game

    // Register the rendering function
    glutDisplayFunc(renderScene);

    // Start the timer and game update functions
    glutTimerFunc(0, updateTimer, 0);  // Start the timer
    glutTimerFunc(16, updateGame, 0);  // Start the game updates

    // Handle special key presses and releases
    glutSpecialFunc(handleSpecialKeyPress);
    glutSpecialUpFunc(handleSpecialKeyRelease);

    glutMainLoop();  // Enter the GLUT main loop

    //return 0;  // Program exit
}
