#version 420

out vec4 outColour;
in vec3 worldPos;

bool is_close(float a, float b, float err){
    return abs(a - b) < err;
}

float f(float p, float x){
    return exp(-p * x);
}

void main() {
    float distance2 = dot(worldPos, worldPos);
    float distanceX = abs(worldPos.x);
    float distanceY = abs(worldPos.y);
    float x = 0.05f;
    float err = 0.001f;
    float p = 50.0f;

    if(is_close(0, mod(distanceX, x), err) || is_close(0, mod(distanceY, x), err)){
        outColour = vec4(1.0f, 1.0f, 1.0f, f(p, distance2));
    }else{
        outColour = vec4(1.0f, 1.0f, 1.0f, 0.0f);
    }

    //outColour = vec4(1.0f, 1.0f, 1.0f, (sin(distance * 200.0f) + 1) / 8);
}
