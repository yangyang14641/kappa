
 // Test for the computation of the R_ci^VT for air5=[N2 NO O2 N O] or simpler mixtures

 #include <map>
 #include <iostream>
 #include <fstream>
 #include <string>
 #include <iomanip> 
 #include <cstdlib>
 #include <vector>
 #include "kappa.hpp"
 #ifdef WINDOWS
 #include <direct.h>
 #define GetCurrentDir _getcwd
 #else
 #include <unistd.h>
 #define GetCurrentDir getcwd
 #endif

 std::string GetCurrentWorkingDir( void ) {
  char buff[FILENAME_MAX];
  GetCurrentDir( buff, FILENAME_MAX );
  std::string current_working_dir(buff);
  return current_working_dir;
 }
 using namespace std;
 using namespace kappa;

 int main(int argc, char** argv) {

// std::cout << "Test for the computation of the R_ci^VT for air5 or simpler mixtures" << std::endl;

 std::string line;
 int linecount=0;
 int _NS=0, _nMol=0, _nAt=0;
 int i=0, j=0, k=0;

 std::vector<std::string> _speciesNames; 

// instantiates molecules and atoms kappa vectors
 std::vector<kappa::Molecule> molecules;
 std::vector<kappa::Atom> atoms;

// instantiates molecular density vector
 std::vector<arma::vec> mol_ndens, mol1_ndens, mol2_ndens;

// instantiates temperature vector
// std::vector<double> T_vals = { 500., 1000., 5000., 10000., 20000., 40000. };

// mixture selection
// std::string _mixtureName = "air5";	// N2 O2 NO N O, ok
// std::string _mixtureName = "air11"; 	// NO+ not in DB
 std::string _mixtureName = "N2";	// N2 + N, ok
// std::string _mixtureName = "O2";	// O2 + O, ok
// std::string _mixtureName = "argon";	// bug 
// std::string _mixtureName = "N2ions";	// N2 N e-, ok
 std::cout << "Mixture: " << _mixtureName << "\n";

//  some path check
 std::string m_libPath = std::getenv("KAPPA_DATA_DIRECTORY");
 std::cout << "KAPPA_DATA_DIRECTORY is: " << m_libPath << '\n';
 std::string particle_source    = m_libPath + "particles.yaml";
 std::string interaction_source = m_libPath + "interaction.yaml";
 std::cout << "particle_source is: " << particle_source << '\n';
 std::cout << "interaction_source is: " << interaction_source << '\n';
 std::string output_dir = GetCurrentWorkingDir();
 std::cout << "Current directory is: " << output_dir << std::endl;

 std::ifstream mixfile((m_libPath+"mixtures/"+_mixtureName+".mix").c_str());

//  open and read input file
 if (mixfile.is_open()) {
  while ( mixfile.good() ) {
   getline(mixfile,line);

   if (line[0] != '/') { // skip lines starting with / 
     linecount+=1;
     if (linecount==1) {
       _NS = static_cast<int> (atoi(line.c_str()));
       _speciesNames.resize(_NS);
     }
     if (linecount>1 && linecount<_NS+2) _speciesNames[linecount-2]=line;
   }
  }
  mixfile.close();
 }
 else {
   std::cout << "Unable to open mixture file: " << m_libPath+"mixtures/"+_mixtureName+".mix" << "\n";
   abort();
 }
 std::cout << "Number of species: " << _NS << "\n";
 std::cout << "Loading particles data ..." << std::endl;

// checks the species vector and create molecules and atoms
 for ( int i = 0; i < _NS; ++i ) {

// Step 1: Check to see if the name is in standard form
	bool standard = isalpha(_speciesNames[i].at(0));
  	if (_speciesNames[i].length() > 1) {
	 for ( j = 0; j < _speciesNames[i].length()-1; ++j) 
           standard = standard && isalnum(_speciesNames[i].at(j));
	   standard = standard && (isalnum(_speciesNames[i].at(j)) || 
				 	   _speciesNames[i].at(j) == '+' || 
				 	   _speciesNames[i].at(j) == '-');
	} // /=1
  	if (!standard) {
	 std::cout << standard << " ouch! " << std::endl;
	 return 0;
	}

 	int length = _speciesNames[i].length();
        std::cout << length << std::endl;

	// Step 2: Now we know that the name is in standard form, parse it to get the stoichiometry
	// Handle the special case of the electron
    	if (_speciesNames[i] == "e-") {
         length = 0;
    	} 
	else if (_speciesNames[i].at(length-1) == '+') {
         length--;
    	} 
	else if (_speciesNames[i].at(length-1) == '-') {
         length--;
    	}
        std::cout << _speciesNames[i] << " " <<  length << std::endl;

        // build the electronic, atomic part of the mixture
        if ( length == 0 || length == 1 ) { // electrons, atoms or atomic ions
	 kappa::Atom at(_speciesNames[i], particle_source);
         atoms.push_back(at);
        } 

        // build the molecular part of the mixture
        else if ( length > 1 ) { // molecules and molecular ions
         kappa::Molecule mol(_speciesNames[i], true, false, particle_source);
         //kappa::Molecule mol(_speciesNames[i], true, true, particle_source);
         molecules.push_back(mol);
         //stox.push_back(mol.stoichiometry[0].second); 
        }
       }  // end for loop on _NS
       std::cout << " Loading particles data completed!" << std::endl;

       _nMol= molecules.size();
       _nAt = atoms.size();
       std::cout << " The mixure has: " << _nMol << " molecules " 
		 << "and " 		<< _nAt  << " atoms" << std::endl;


// instantiates the mixture object and some print info
 kappa::Mixture mixture(molecules, atoms, interaction_source, particle_source);

 std::cout << " The mixture has been created! " << std::endl;
 std::string MixtureComponents = mixture.get_names();
 std::cout << " Mixture's components: " << MixtureComponents << std::endl;
 std::cout << " Number of component species: " << mixture.get_n_particles() << std::endl;
 int nvls = mixture.get_n_vibr_levels();
 std::cout << " Mix. tot. n. of vibr. ls.: " << nvls << std::endl;
 std::vector<int> vls;
 vls = mixture.get_n_vibr_levels_array();
 for ( auto i = vls.begin(); i != vls.end(); i++ ) {
    std::cout << *i << std::endl;   
 }

// define the level of approximation
 Approximation appr;

// std::vector<double> T_vals = { 500., 1000., 2000., 5000., 10000., 15000., 20000., 25000., 30000., 35000., 40000. };
 std::vector<double> T_vals = { 5000., 10000., 15000. };
// std::vector<int> i_vals;
// i_vals = { 1, 5, 10, 20, 30 };

 // kappa::models_k_vt model_k_vt = kappa::models_k_vt::model_k_vt_rs_fho; // FIXME buggy
 // kappa::models_k_vt model_k_vt = kappa::models_k_vt::model_k_vt_vss_fho; // FIXME buggy
 kappa::models_k_vt model_k_vt = kappa::models_k_vt::model_k_vt_ssh; // FIXME strange results, probably buggy
 // kappa::models_k_vt model_k_vt = kappa::models_k_vt::model_k_vt_phys4entry; // FIXME missing data
 // kappa::models_k_vt model_k_vt = kappa::models_k_vt::model_k_vt_billing; // seems ok!


 double T = T_vals[0];
 double nd;
      
// molecule-molecule
 std::vector<double> R_VT, R_VT_at, R_VT_mol;
 std::vector<double> k_VT;
 int mol_count = 0, mol1_count = 0, mol2_count = 0;
 double k_VT_mol_ip1_mol_i = 0.;
 double k_VT_mol_i_mol_ip1 = 0.;
 double k_VT_mol_im1_mol_i = 0.;
 double k_VT_mol_i_mol_im1 = 0.;

 arma::vec atom_ndens = arma::zeros(1);
 double tot_ndens =  101325.0 / (kappa::K_CONST_K * T);
 double x_atom; 
 int atom_count = 0;
 mol_count = 0;
 x_atom = 0.10;
 atom_ndens[0] = x_atom * tot_ndens;

// for (i=0; i<atom_ndens.n_cols; ++i) {
//  std::cout << " atom_ndens " << atom_ndens[i] << std::endl;
// } 

 std::ofstream outf1;
 outf1.open(output_dir + "/k_VT_" + _mixtureName + ".txt");
 outf1 << std::setw(20) << "T [K]";
 outf1 << std::setw(20) << "Vibr. level";
 outf1 << std::setw(20) << "k_VT_mol_ip1_mol_i";
 outf1 << std::setw(20) << "k_VT_mol_i_mol_ip1";
 outf1 << std::setw(20) << "k_VT_mol_im1_mol_i";
 outf1 << std::setw(20) << "k_VT_mol_i_mol_im1";
 outf1 << std::endl;   

 double R_VT_moll = 0.;
 double R_VT_att = 0.;

 for (auto mol1 : molecules) { // c

  mol1_ndens.push_back(mixture.Boltzmann_distribution(T, (1.-x_atom)*tot_ndens, mol1)); // first molecule
  mol1_ndens[0] = mixture.Boltzmann_distribution(T, (1.-x_atom)*tot_ndens, mol1);

  for (i=0; i<mol1.num_vibr_levels[0]; ++i) { // i = 0, ... l-1

   // molecule + molecule
   for (auto mol2 : molecules) { // M or d -- second molecule

    mol2_ndens.push_back(mixture.Boltzmann_distribution(T, (1.-x_atom)*tot_ndens, mol2));
    mol2_ndens[0] = mixture.Boltzmann_distribution(T, (1.-x_atom)*tot_ndens, mol2);
    nd = mixture.compute_n(mol2_ndens[mol2_count]);  

    kappa::Interaction inter_mol_mol(mol1, mol2, interaction_source); 

    //std::cout << " i = " << i << " of " << mol1.num_vibr_levels[0] << std::endl;
    if (i == 0) { // first vibrational level
      k_VT_mol_ip1_mol_i = appr.k_VT(T, mol1, inter_mol_mol, i+1, -1, model_k_vt); // i+1 -> i (down)
      k_VT_mol_i_mol_ip1 = appr.k_VT(T, mol1, inter_mol_mol, i  ,  1, model_k_vt); // i -> i+1 (up)
    }
    else if (i > 0 && i < mol1.num_vibr_levels[0]-1) {
      k_VT_mol_ip1_mol_i = appr.k_VT(T, mol1, inter_mol_mol, i+1, -1, model_k_vt); // i+1 -> i (down)
      k_VT_mol_i_mol_ip1 = appr.k_VT(T, mol1, inter_mol_mol, i  ,  1, model_k_vt); // i -> i+1 (up)
      k_VT_mol_im1_mol_i = appr.k_VT(T, mol1, inter_mol_mol, i-1,  1, model_k_vt); // i-1 -> i (up)
      k_VT_mol_i_mol_im1 = appr.k_VT(T, mol1, inter_mol_mol, i  , -1, model_k_vt); // i -> i-1 (down)
    }
    else if (i == mol1.num_vibr_levels[0]-1) { // last vibrational level
      k_VT_mol_im1_mol_i = appr.k_VT(T, mol1, inter_mol_mol, i-1,  1, model_k_vt); // i-1 -> i (up)
      k_VT_mol_i_mol_im1 = appr.k_VT(T, mol1, inter_mol_mol, i  , -1, model_k_vt); // i -> i-1 (down)
    } 
    else {
      std::cout << " Error, something wrong! Probably some out-of-bound shit! " << std::endl;
      abort();
    }

    // print screen of VT rate exchange coefficients for molecule-molecule interaction
    std::cout << " molecules " << std::endl;
    std::cout << i << " " << k_VT_mol_ip1_mol_i << " " 
			  << k_VT_mol_i_mol_ip1 << " " 
			  << k_VT_mol_im1_mol_i << " " 
			  << k_VT_mol_i_mol_im1 << std::endl;
    std::cout << " " << std::endl;

    outf1 << std::setw(20) << T;
    outf1 << std::setw(20) << i;
    outf1 << std::setw(20) << k_VT_mol_ip1_mol_i;
    outf1 << std::setw(20) << k_VT_mol_i_mol_ip1;
    outf1 << std::setw(20) << k_VT_mol_im1_mol_i;
    outf1 << std::setw(20) << k_VT_mol_i_mol_im1;
    //outf1 << std::endl;   

    
    if (i == 0) { // first vibrational level
      R_VT_moll += (nd * (mol1_ndens[mol_count].at(i+1)*k_VT_mol_ip1_mol_i - mol1_ndens[mol_count].at(i)*k_VT_mol_i_mol_ip1));
    }
    else if (i > 0 && i < mol1.num_vibr_levels[0]-1) {
      R_VT_moll += (nd * (mol1_ndens[mol_count].at(i+1)*k_VT_mol_ip1_mol_i - mol1_ndens[mol_count].at(i)*k_VT_mol_i_mol_ip1 + 
                          mol1_ndens[mol_count].at(i-1)*k_VT_mol_im1_mol_i - mol1_ndens[mol_count].at(i)*k_VT_mol_i_mol_im1));
    } 
    else if (i == mol1.num_vibr_levels[0]-1) { // last vibrational level
      R_VT_moll += (nd * (mol1_ndens[mol_count].at(i-1)*k_VT_mol_im1_mol_i - mol1_ndens[mol_count].at(i)*k_VT_mol_i_mol_im1));
    }
    else {
      std::cout << " Error, something wrong! Probably some out-of-bound shit! " << std::endl;
      abort();
    }

    std::cout << " R_VT_mol = " << R_VT_moll << std::endl;
    std::cout << " mol2_count = " << mol2_count << std::endl;
    mol2_count++;
   } // end molecule + molecule

 
   // molecule + atom  
   atom_count = 0;
   for (auto at1 : atoms) {
     
    kappa::Interaction inter_mol_at(mol1, at1, interaction_source);

    if (i == 0) {
     k_VT_mol_ip1_mol_i = appr.k_VT(T, mol1, inter_mol_at, i+1, -1, model_k_vt); 
     k_VT_mol_i_mol_ip1 = appr.k_VT(T, mol1, inter_mol_at, i  ,  1, model_k_vt); 
    }
    else if (i > 0 && i < mol1.num_vibr_levels[0]-1) {
     k_VT_mol_ip1_mol_i = appr.k_VT(T, mol1, inter_mol_at, i+1, -1, model_k_vt); 
     k_VT_mol_i_mol_ip1 = appr.k_VT(T, mol1, inter_mol_at, i  ,  1, model_k_vt); 
     k_VT_mol_im1_mol_i = appr.k_VT(T, mol1, inter_mol_at, i-1,  1, model_k_vt); 
     k_VT_mol_i_mol_im1 = appr.k_VT(T, mol1, inter_mol_at, i  , -1, model_k_vt); 
    }
    else if (i == mol1.num_vibr_levels[0]-1) {
     k_VT_mol_im1_mol_i = appr.k_VT(T, mol1, inter_mol_at, i-1,  1, model_k_vt); 
     k_VT_mol_i_mol_im1 = appr.k_VT(T, mol1, inter_mol_at, i ,  -1, model_k_vt); 
    } 
    else {
     std::cout << " Error, something wrong! Probably out-of-bound error." << std::endl;
     abort();
    }

    // print screen of VT rate exchange coefficients for molecule-atom interaction
    std::cout << " atoms " << std::endl;
    std::cout << i << " " << k_VT_mol_ip1_mol_i << " " << k_VT_mol_i_mol_ip1 << " " << k_VT_mol_im1_mol_i << " " << k_VT_mol_i_mol_im1 << std::endl;
    std::cout << " " << std::endl;
    outf1 << std::setw(20) << k_VT_mol_ip1_mol_i;
    outf1 << std::setw(20) << k_VT_mol_i_mol_ip1;
    outf1 << std::setw(20) << k_VT_mol_im1_mol_i;
    outf1 << std::setw(20) << k_VT_mol_i_mol_im1;
    outf1 << std::endl;   

    std::cout << " atom_ndens[atom_count] = " << atom_ndens[atom_count] << std::endl;
    if (i == 0) { // first vibrational level
     R_VT_att += ( atom_ndens[atom_count] * (mol1_ndens[mol_count].at(i+1)*k_VT_mol_ip1_mol_i - mol1_ndens[mol_count].at(i)*k_VT_mol_i_mol_ip1));
    }
    else if (i > 0 && i < mol1.num_vibr_levels[0]-1) {
     R_VT_att += ( atom_ndens[atom_count] * (mol1_ndens[mol_count].at(i+1)*k_VT_mol_ip1_mol_i - mol1_ndens[mol_count].at(i)*k_VT_mol_i_mol_ip1 + 
                                             mol1_ndens[mol_count].at(i-1)*k_VT_mol_im1_mol_i - mol1_ndens[mol_count].at(i)*k_VT_mol_i_mol_im1));
    } 
    else if (i == mol1.num_vibr_levels[0]-1) { // last vibrational level
     R_VT_att += ( atom_ndens[atom_count] * (mol1_ndens[mol_count].at(i-1)*k_VT_mol_im1_mol_i - mol1_ndens[mol_count].at(i)*k_VT_mol_i_mol_im1));
    }
    else {
     std::cout << " Error, something wrong! Probably some out-of-bound shit! " << std::endl;
     abort();
    }

    std::cout << " R_VT_at = " <<  R_VT_att << std::endl;
    std::cout << " atom_count = " << atom_count << std::endl;
    atom_count++;
   } // end molecule + atom

   std::cout << " R_VT_mol + R_VT_at = " << R_VT_moll << "   " <<  R_VT_att << std::endl;
   R_VT.push_back(R_VT_moll + R_VT_att);
  } // end vibrational levels c,i
  mol_count++;
 } // end mol1, c

 std::cout << " R_VT " << std::endl;
 int cc = 0;
 for ( auto i = R_VT.begin(); i != R_VT.end(); i++ ) {
  std::cout << cc << " " << (*i) << std::endl;
  cc++;
 }

// print output file
 int counter = 0;
 std::ofstream outf;
 outf.open(output_dir + "/R_VT_" + _mixtureName + ".txt");
 outf << std::setw(20) << "T [K]";
 outf << std::setw(20) << "Vibr. level";
 outf << std::setw(20) << "R_VT";
 outf << std::endl;   
 for ( auto i = R_VT.begin(); i != R_VT.end(); i++ ) {
  std::cout << *i << std::endl;   
  outf << std::setw(20) << T;
  outf << std::setw(20) << counter;
  outf << std::setw(20) << *i;
  outf << std::endl;   
  counter++;
 }

} // end main