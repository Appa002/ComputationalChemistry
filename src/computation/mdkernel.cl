__constant float SCALING = 0.00001f; //random scaling factor TODO: DO PROPER

void electro_static_force (float deltaTime, __global float * charges, __global float* momenta, __global float* positions,
                           __global float* cf_x, __global float* cf_y, __global float* cf_z, int N){

    const int i = get_global_id(0);
    const int j = get_global_id(1);

    const float qi = charges[i];
    const float pi[3] =  {positions[i], positions[i+N], positions[i+2*N]};

    float r_2 = pow(pi[0]-positions[j],2) + pow(pi[1]-positions[j+N],2) + pow(pi[2]-positions[j+2*N],2); // r^2 = a^2 + b^2 +c^2
    r_2 *= SCALING;

    if(r_2 < 0.00001f)
        return; // If the two charges are very close, then we skip this simulation step. This is not particularly accurate but better than devision by zero.

    float force_mag = SCALING * (qi * charges[j]) / r_2;

    float force_ji_x = pi[0] - positions[j];     // poynting vec x
    float force_ji_y = pi[1] - positions[j+N];   // poynting vec y
    float force_ji_z = pi[2] - positions[j+2*N]; // poynting vec z

    // Normalizing ...
    float abs = sqrt(r_2);
    force_ji_x /= abs;
    force_ji_y /= abs;
    force_ji_z /= abs;

    // Scaling ...
    force_ji_x *= force_mag;
    force_ji_y *= force_mag;
    force_ji_z *= force_mag;

    // Adding to cumulative force...

    cf_x[i] += force_ji_x;
    cf_y[i] += force_ji_y;
    cf_z[i] += force_ji_z;

}



inline void apply_position_and_momentum (float deltaTime, __global float * out_positions, __global float* positions, __global float* masses, const int i,
                                  float cf_x, float cf_y, float cf_z, const int N, const float factor){

    const float next_velocity[3] = {cf_x/masses[i] * deltaTime * factor, cf_y/masses[i] * deltaTime * factor, cf_z/masses[i] * deltaTime * factor};

    out_positions[i] += next_velocity[0];
    out_positions[i+N] += next_velocity[1];
    out_positions[i+2*N] += next_velocity[2];
}

inline float3 get_orth_norm_vec(float3 vec, float3 ref){
    float3 temp = vec - ref * dot(vec, ref);
    return normalize(temp);
}

__kernel void bond_stretch(float deltaTime, __global float* out_positions, __global float* masses,
                           __global float* positions, __global float* bonds, int N){

    int i = get_global_id(0);
    // Unpacking data structure...
    __global float* descriptor = &bonds[ (int)bonds[i] ];
    float k = descriptor[0];
    float r0 = descriptor[1];
    float D = descriptor[2];
    int atome_one = (int)descriptor[3];
    int atome_two = (int)descriptor[4];

    // Calculating energy...

    float alpha = sqrt(k / (2 * D));

    float r_2 = pow(positions[atome_one] -positions[atome_two],2) + pow(positions[atome_one+N]-positions[atome_two+N],2) + pow(positions[atome_one+2*N]-positions[atome_two+2*N],2); // r^2 = a^2 + b^2 +c^2
    float r = sqrt(r_2);

    float energy = D * pow( exp(-1 * alpha * (r - r0)) - 1, 2);

    // Differentiating...
    const float h = 0.000001f;

    float energy_with_h = D * pow( exp(-1 * alpha * ((r+h) - r0)) - 1, 2);;

    float f_mag = (energy_with_h - energy) / h;

    // The force due to the energy will point such that r -> r0
    // Calculating force vector...
    float f_x_ji = positions[atome_one] -positions[atome_two];
    float f_y_ji = positions[atome_one+N] -positions[atome_two+N];
    float f_z_ji = positions[atome_one+2*N] -positions[atome_two+2*N];

    //Normalizing...
    f_x_ji /= r;
    f_y_ji /= r;
    f_z_ji /= r;

    // Scaling...
    f_x_ji *= -f_mag;
    f_y_ji *= -f_mag;
    f_z_ji *= -f_mag;

    // Apply...
    apply_position_and_momentum(deltaTime, out_positions, positions, masses, atome_one, f_x_ji, f_y_ji, f_z_ji, N, 1.0f);
    apply_position_and_momentum(deltaTime, out_positions, positions, masses, atome_two, f_x_ji, f_y_ji, f_z_ji, N, -1.0f);

}

__kernel void angle_bend(float deltaTime, __global float * out_positions,__global float* positions, __global float* angles, __global float* masses, int N){

    const int i = get_global_id(0);

    /* Unpacking data structure... */
    __global float* description = &angles[ (int)angles[i] ];

    float kijk = description[0];
    float natural_angle = description[1];
    int atom_centre = description[2];
    int c_n = (int)description[3];
    __global float *cs = &description[4];
    int atom_one = (int)description[4+c_n];
    int atom_two = (int)description[5+c_n];

    /* Finding angle... */

    float one_x = positions[atom_one];
    float one_y = positions[atom_one+N];
    float one_z = positions[atom_one+2*N];

    float two_x = positions[atom_two];
    float two_y = positions[atom_two+N];
    float two_z = positions[atom_two+2*N];

    float centre_x = positions[atom_centre];
    float centre_y = positions[atom_centre+N];
    float centre_z = positions[atom_centre+2*N];

    float rone_2 = pow(one_x - centre_x, 2) + pow(one_y - centre_y, 2) + pow(one_z - centre_z, 2); // square of distance from centre atom to first atom
    float rtwo_2 = pow(two_x - centre_x, 2) + pow(two_y - centre_y, 2) + pow(two_z - centre_z, 2); // square of distance from centre atom to second atom
    float ronetwo_2 = pow(two_x -one_x, 2) + pow(two_y - one_y, 2) + pow(two_z - one_z, 2); // square of distance form first atom to second atom
    float ronetwo = sqrt(ronetwo_2);

    float cos_theta = (ronetwo_2 - rone_2 - rtwo_2) / (-2.0f*ronetwo); // using cosine rule ; cos(theta)
    float theta = acos(cos_theta); // angle between the three atoms

    /* Finding energy due to valance angle bending... */
    /* ... at theta ... */
    float fourier = 0.0f;
    for (int n = 0; n < c_n; n++){
        fourier += cs[n] * cos(n*theta);
    }
    float E = kijk * fourier;

    /* ... at theta + h ...*/
    const float h = 0.00001f;
    fourier = 0.0f;
    for (int i = 0; i < c_n; i++){
        fourier += cs[i] * cos(i*(theta+h));
    }

    float E_h = kijk * fourier;

    /* Differentiating... */
    float force_mag = (E_h - E) / h;
    force_mag *= -1.0f;

    /* Finding force vector... */
    float cf_x = two_x - one_x;
    float cf_y = two_y - one_y;
    float cf_z = two_z - one_z;

    /*... normalizing ... */
    cf_x /= ronetwo;
    cf_y /= ronetwo;
    cf_z /= ronetwo;

    /*...scaling ...*/
    cf_x *= force_mag;
    cf_y *= force_mag;
    cf_z *= force_mag;

    /* Applying position and momentum changes to atom one and atom two... */
  //  apply_position_and_momentum(deltaTime, out_positions, positions, masses, atom_one, cf_x, cf_y, cf_z, N, 1.0f);
  //  apply_position_and_momentum(deltaTime, out_positions, positions, masses, atom_two, cf_x, cf_y, cf_z, N, -1.0f);

}

__kernel void torsion_bend(float deltaTime, __global float* out_positions, __global float* masses, __global float* positions, __global float* torsions, int N){

    const int i = get_global_id(0);

    // Unpacking data structure...
    __global float* description = &torsions[(int)torsions[i]];
    float v = description[0];
    float n = description[1];
    float firstExpansion = description[2];
    int atom_inner_one = (int) description[3];
    int atom_inner_two = (int) description[4];
    int atom_outer_one = (int) description[5];
    int atom_outer_two = (int) description[6];

    // Finding angle...
    float3 ione = (float3) ( positions[atom_inner_one], positions[atom_inner_one + N], positions[atom_inner_one + 2*N] ); // position vector of atom inner one
    float3 itwo = (float3) ( positions[atom_inner_two], positions[atom_inner_two + N], positions[atom_inner_two + 2*N] );

    float3 oone = (float3) ( positions[atom_outer_one], positions[atom_outer_one + N], positions[atom_outer_one + 2*N] );
    float3 otwo = (float3) ( positions[atom_outer_two], positions[atom_outer_two + N], positions[atom_outer_two + 2*N] );

    float3 ione_itwo = normalize(itwo - ione);

    float3 plane_one = get_orth_norm_vec(oone - ione, ione_itwo);
    float3 plane_two = get_orth_norm_vec(otwo - itwo, ione_itwo);

    float angle = acos(dot(plane_one, plane_two));
    float sign = dot(cross(plane_one, plane_two), ione_itwo);
    angle *= sign < 0 ? (-1.0f) : (1.0f);

    // Finding energy...
    float energy = 0.5f * v * (1 - firstExpansion * cos(n*angle));

    // Differentiating...
    const float h = 0.00001f;

    float energy_h =  0.5f * v * (1 - firstExpansion * cos(n*(angle+h)));

    float force_mag = (energy_h - energy) / h;

    // Finding force change
    float3 poynting = plane_two - plane_one; // already normalized
    poynting *= force_mag;

    // Apply it...
    apply_position_and_momentum(deltaTime, out_positions, positions, masses, atom_outer_one, poynting.s0, poynting.s1, poynting.s2, N, 1.0f);
    apply_position_and_momentum(deltaTime, out_positions, positions, masses, atom_outer_two, poynting.s0, poynting.s1, poynting.s2, N, -1.0f);

}





















