package com.example.labstatus.ui.CS101;

import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.ColorStateList;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ProgressBar;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProviders;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.example.labstatus.DetailActivity;
import com.example.labstatus.MainActivity;
import com.example.labstatus.R;
import com.google.android.material.bottomnavigation.BottomNavigationView;
import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.google.android.material.snackbar.Snackbar;


import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.concurrent.TimeUnit;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

public class CS101Fragment extends Fragment implements RecyclerViewAdapterBasement.ItemListener {

    private CS101ViewModel CS101ViewModel;
    RecyclerView recyclerView;
    ArrayList arrayList;
    ProgressBar bar;
    AlertDialog.Builder builder;

    int ShutDownFlag=1;
    View root;
    public String TAG = CS101Fragment.class.getSimpleName();
    HashMap<String, Boolean> status=new HashMap<>();
    String CS101_ini="10.130.152.";
    RecyclerViewAdapterBasement adapter;
    BottomNavigationView navView;
    int keyboard_request=0;
    int shutdown_request=0;
    int mouse_request=0;
    int lab_status=0;

    String ServerIP="10.130.150.223";
    int port=5000;
    String lab="cs101";

    String url = "http://"+ServerIP+":"+port +"/lab/";




    boolean isFABOpen=false;
    FloatingActionButton fab_lab,fab_keyboard,fab_mouse,fab,fab_wakeup,fab_refresh;

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
        CS101ViewModel =
                ViewModelProviders.of(this).get(CS101ViewModel.class);
        root = inflater.inflate(R.layout.fragment_notifications, container, false);
        bar=root.findViewById(R.id.pBar_CS101);
        bar.setVisibility(root.GONE);
        builder = new AlertDialog.Builder(getActivity());

        recyclerView = root.findViewById(R.id.recyclerViewBasement);
        arrayList = new ArrayList();
        fab =  root.findViewById(R.id.fab_cs101);
        fab_lab =  root.findViewById(R.id.fab_lab_cs101);
        fab_keyboard =  root.findViewById(R.id.fab_keyboard_cs101);
        fab_mouse =  root.findViewById(R.id.fab_mouse_cs101);
        fab_wakeup= root.findViewById(R.id.fab_wakeup_cs101);
        fab_wakeup=root.findViewById(R.id.fab_wakeup_cs101);
        fab_refresh=root.findViewById(R.id.fab_cs101_refresh);
        navView=((MainActivity)this.getActivity()).getNavView();
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(!isFABOpen){
                    showFABMenu();
                }else{
                    closeFABMenu();
                }
            }
        });
        fab_refresh.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v) {
                if (lab_status == 0)
                {
                    lab_status=1;
                    fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                    fab.setImageResource(R.drawable.ic_refresh);
                    fab_wakeup.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    fab_refresh.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                    fab_keyboard.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    fab_wakeup.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    fab_lab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    fab_mouse.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    try {
                        GetLabStatus();
                    } catch (Exception e) {
                        Log.e(TAG, e.toString());
                    }
                }
                else{
                    showSnackBarMessage("Please wait for previous request completion");
                }
            }
        });


        fab_wakeup.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                fab.setImageResource(R.drawable.ic_wakeup);
                fab_wakeup.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));

                fab_keyboard.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                fab_wakeup.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                fab_lab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                fab_mouse.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                WakeOnLAN();
                // new WakeOnLAN().execute();

            }
        });
        fab_lab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (shutdown_request == 0)
                {
                    shutdown_request=1;
                    fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                    fab.setImageResource(R.drawable.ic_lab);
                    fab_wakeup.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));


                    fab_keyboard.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    fab_mouse.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));


                    builder.setMessage("Do you want to shut down all pc of CS101 Lab?")
                            .setCancelable(false)
                            .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int id) {
                                    fab_lab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.color_hardware)));
                                    ShutDownLab();
                                    //new ShutDownLab().execute();
                                }
                            })
                            .setNegativeButton("No", new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int id) {
                                    fab_lab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                                    dialog.cancel();

                                }
                            });

                    AlertDialog alert = builder.create();
                    alert.setTitle("Shut down");
                    alert.show();
                }
                else{
                    showSnackBarMessage("Please wait for previous request completion");
                }



            }
        });
        fab_keyboard.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (keyboard_request == 0){
                    keyboard_request=1;
                    fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                    fab.setImageResource(R.drawable.ic_keyboard);
                    fab_wakeup.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));

                    fab_keyboard.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                    fab_lab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    fab_mouse.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    GetKeyboardStatus();
                    //new GetKeyboardStatus().execute();
                }
                else{
                    showSnackBarMessage("Please wait for previous request completion");
                }
            }
        });
        fab_mouse.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mouse_request == 0)
                {
                    mouse_request=1;
                    fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                    fab.setImageResource(R.drawable.ic_mouse);
                    fab_wakeup.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));

                    fab_mouse.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                    fab_keyboard.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    fab_lab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorAccent)));
                    GetMouseStatus();
                    //new GetMouseStatus().execute();
                }
                else{
                    showSnackBarMessage("Please wait for previous request completion");
                }
            }
        });



        arrayList = new ArrayList();
        //new GetLabStatus().execute();
        try {
            GetLabStatus();
        }
        catch (Exception e){
            Log.e(TAG,e.toString());
        }

        return root;
    }

    /**
     * Expands the floating actions button which are stacked over one another.
     */
    private void showFABMenu(){
        isFABOpen=true;
        fab.setImageResource(R.drawable.ic_fab_right);
        fab_wakeup.animate().translationX(-getResources().getDimension(R.dimen.standard_65));

        fab_lab.animate().translationX(-getResources().getDimension(R.dimen.standard_115));

        fab_keyboard.animate().translationX(-getResources().getDimension(R.dimen.standard_165));
        fab_mouse.animate().translationX(-getResources().getDimension(R.dimen.standard_215));
        fab_refresh.animate().translationX(-getResources().getDimension(R.dimen.standard_265));
    }
    /**
     * Closes the expanded floating actions button.
     */
    private void closeFABMenu(){
        isFABOpen=false;
        // fab.setImageResource(R.drawable.ic_fab_left);
        fab_lab.animate().translationX(0);
        fab_wakeup.animate().translationX(0);
        fab_keyboard.animate().translationX(0);
        fab_mouse.animate().translationX(0);
        fab_refresh.animate().translationX(0);
    }

    @Override
    public void onItemClick(CS101ViewModel item) {
        try {
            //Toast.makeText(getContext(), item.text + " is clicked", Toast.LENGTH_SHORT).show();
            Intent intent = new Intent(getActivity(), DetailActivity.class);
            intent.putExtra("lab", "cs101");
            intent.putExtra("pc", item.text);

            if (status.get(CS101_ini + item.text) == false)
                Snackbar.make(root, "System offline", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();
            else {
                startActivity(intent);
            }
        }
        catch(Exception e){
            Log.e(TAG,e.toString());
        }
    }
    /**
     * Gets the lab status of respective Lab.
     * @throws IOException
     */
    private void GetLabStatus() throws IOException
    {

        preprocessing_task();
        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(300,TimeUnit.SECONDS)
                .writeTimeout(300,TimeUnit.SECONDS)
                .readTimeout(300,TimeUnit.SECONDS)
                .build();
        String urltemp=url;
        urltemp+=lab+"/status";
        Log.e(TAG,"Sending request :"+urltemp);
        //  String url = "http://10.130.150.223:5000/lab/CS101/status";
        Request request = new Request.Builder()
                .url(urltemp)
                .build();

        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                call.cancel();
                Log.e(TAG,e.toString());
            }

            @Override
            public void onResponse(Call call, final Response response) throws IOException {
                try {
                    int httpResponseCode = response.code();
                    Log.e(TAG, "resp code=" + httpResponseCode);
                    if (httpResponseCode == 200) {
                        final String myResponse = response.body().string();

                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {

                                get_info(myResponse);
                                update_UI("status");
                                post_processing_task();
                                lab_status = 0;


                            }
                        });
                    } else {
                        showSnackBarMessage("Could not connect with server");
                        post_processing_task();
                    }
                }
                catch(Exception e1)
                {
                    Log.e(TAG, e1.toString());
                }
            }
        });
    }
    /**
     * Hides the progress bar
     */
    private void post_processing_task(){
        try {
            getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    try {
                        bar.setVisibility(root.GONE);
                    } catch (Exception e) {
                        Log.e(TAG, e.toString());
                    }
                }
            });
        }
        catch(Exception e){
            Log.e(TAG,e.toString());
        }
    }
    /**
     * This will print the passesd parameter msg in snackbar.
     * @param msg - message to be printed
     */
    private void showSnackBarMessage(String msg) {
        try {
            Log.e(TAG, msg);
            final String temp = msg;
            getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    try {
                        Snackbar.make(root, temp, Snackbar.LENGTH_LONG)
                                .setAction("Action", null).show();
                    } catch (Exception e) {
                        Log.e(TAG, e.toString());
                    }
                }
            });
        }
        catch (Exception e){
            Log.e(TAG,e.toString());
        }
    }

    /**
     * This will show the progress bar.
     */
    private void preprocessing_task() {
        try {
            Log.e(TAG, "Performing preprocessing task");
            getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    try {
                        bar.setVisibility(root.VISIBLE);
                    } catch (Exception e) {
                        Log.e(TAG, e.toString());
                    }
                }
            });
            arrayList.clear();
            status.clear();
        }
        catch(Exception e ){
            Log.e(TAG,e.toString());
        }
    }

    /**
     * This will update the recycerView GridView according to the operation specified
     * @param operation The openation according thorugh which we need to update the UI
     */
    private void update_UI(String operation) {
        try {
            String color="#008577";
            String error_color="#D81B60";

            if(operation=="keyboard" || operation == "mouse"){

                error_color ="#927B01";
            }
            int counter = 1;
            String label;
            Integer no;
            for (int i = -1; i < 13; i++) {
                for (int j = 1; j <= 15; j++) {

                    if (i == -1) {
                        if (j == 8) {
                            no = 200;
                            label = no.toString();
                        } else {
                            label = "";
                        }
                    } else if (i == 0 && (j == 1 || j == 2 || j == 14 || j == 15)) {
                        label = "";
                    } else if (i == 1 && (j == 1 || j == 15)) {
                        label = "";
                    } else if (i == 2 && (j == 1 || j == 15)) {
                        label = "";
                    } else {
                        if (j == 5 || j == 11) {
                            label = "";
                        } else {


                            no = counter;
                            counter++;
                            if (no > 148) {
                                label = "";
                            } else {
                                label = no.toString();
                            }
                           /* no = (14 * i) + j;
                            if(no>132){
                                label="";
                            }*/


                        }
                    }

                    if (label != "") {
                        String ip = CS101_ini + label;
                        boolean temp = false;
                        if (status.get(ip) != null) {
                            temp = status.get(ip);
                        }

                        if (temp == true) {
                            arrayList.add(new CS101ViewModel(label, color));
                        } else {
                            arrayList.add(new CS101ViewModel(label, error_color));
                        }
                    } else {
                        arrayList.add(new CS101ViewModel(label, error_color));
                    }
                }
            }
            recyclerView.invalidate();
             adapter = new RecyclerViewAdapterBasement(getContext(), arrayList, CS101Fragment.this);
            recyclerView.setAdapter(adapter);


            /**
             AutoFitGridLayoutManager that auto fits the cells by the column width defined.
             **/

        /*AutoFitGridLayoutManager layoutManager = new AutoFitGridLayoutManager(this, 500);
        recyclerView.setLayoutManager(layoutManager);*/


            /**
             Simple GridLayoutManager that spans two columns
             **/
            GridLayoutManager manager = new GridLayoutManager(getContext(), 15, GridLayoutManager.VERTICAL, false);
            recyclerView.setLayoutManager(manager);
            getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    bar.setVisibility(root.GONE);
                }
            });
        }
        catch(Exception e){
            Log.e(TAG,e.getMessage());
        }
    }
    /**
     * Gets the Keyboard Status via OKHttp Asynchronously
     */
    private void GetKeyboardStatus(){
        preprocessing_task();

        // ena_dis_UI_elements(false);
        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(300,TimeUnit.SECONDS)
                .writeTimeout(300,TimeUnit.SECONDS)
                .readTimeout(300,TimeUnit.SECONDS)
                .build();

        String urltemp=url;
        urltemp+=lab+"/hardware/keyboard";
        Log.e(TAG,"Sending request :"+urltemp);
        //  String url = "http://10.130.150.223:5000/lab/CS101/status";
        Request request = new Request.Builder()
                .url(urltemp)
                .build();
        showSnackBarMessage("Request Send.Please wait ...");
        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                call.cancel();
                showSnackBarMessage("Could not connect with server");
                post_processing_task();
                //ena_dis_UI_elements(true);
                keyboard_request=0;
            }

            @Override
            public void onResponse(Call call, final Response response) throws IOException {
                try{
                    int httpResponseCode=response.code();
                    if (httpResponseCode == 200) {
                        final String myResponse = response.body().string();

                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {

                                get_info(myResponse);
                                update_UI("keyboard");
                                //post_processing_task();
                                //  ena_dis_UI_elements(true);
                                keyboard_request=0;
                                closeFABMenu();
                                fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                                fab.setImageResource(R.drawable.ic_keyboard);

                            }
                        });
                    }
                    else{
                        showSnackBarMessage("Could not connect with server");

                        // ena_dis_UI_elements(true);
                        keyboard_request=0;
                    }
                    post_processing_task();
                }

                catch(Exception e1){
                    Log.e(TAG,e1.toString());
                }
            }
        });
    }


    private void get_info(String response) {
        arrayList.clear();
        status.clear();
        if (response != null) {
            try {
                JSONObject jsonObj = new JSONObject(response);


                Iterator<String> iter = jsonObj.keys();
                while (iter.hasNext()) {
                    String key = iter.next();
                    try {
                        Boolean value = (Boolean) jsonObj.get(key);
                        status.put(key,value);
                    } catch (JSONException e) {
                        // Something went wrong!
                    }
                }



            } catch (final JSONException e) {
                Log.e(TAG, "Json parsing error: " + e.getMessage());

            }

        } else {

            showSnackBarMessage("Could not connect with server");
            post_processing_task();
        }
    }
    /**
     * Gets the Mouse Status via OKHttp Asynchronously
     */
    private void GetMouseStatus(){
        preprocessing_task();

        //ena_dis_UI_elements(false);
        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(300,TimeUnit.SECONDS)
                .writeTimeout(300,TimeUnit.SECONDS)
                .readTimeout(300,TimeUnit.SECONDS)
                .build();

        String urltemp=url;
        urltemp+=lab+"/hardware/mouse";
        Log.e(TAG,"Sending request :"+urltemp);
        //  String url = "http://10.130.150.223:5000/lab/CS101/status";
        final Request request = new Request.Builder()
                .url(urltemp)
                .build();
        showSnackBarMessage("Request Send.Please wait ...");
        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                try {
                    call.cancel();
                    showSnackBarMessage("Could not connect with server");
                    post_processing_task();
                    //  ena_dis_UI_elements(true);
                    mouse_request = 0;
                    post_processing_task();
                }
                catch(Exception e1){
                    Log.e(TAG, e1.toString());
                }
            }

            @Override
            public void onResponse(Call call, final Response response) throws IOException {
                try {
                    int httpResponseCode = response.code();
                    if (httpResponseCode == 200) {
                        final String myResponse = response.body().string();

                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {

                                get_info(myResponse);
                                update_UI("mouse");
                                //post_processing_task();
                                //   ena_dis_UI_elements(true);
                                mouse_request = 0;
                                closeFABMenu();
                                fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                                fab.setImageResource(R.drawable.ic_mouse);

                            }
                        });
                    } else {
                        showSnackBarMessage("Could not connect with server");

                        //  ena_dis_UI_elements(true);
                        mouse_request = 0;
                    }
                    post_processing_task();
                }
                catch(Exception e){
                    Log.e(TAG,e.toString());
                }
            }
        });
    }

    /**
     * Turn on all the PC's of a Lab via WakeOnLan
     */
    private void WakeOnLAN(){
        preprocessing_task();

        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(300,TimeUnit.SECONDS)
                .writeTimeout(300,TimeUnit.SECONDS)
                .readTimeout(300,TimeUnit.SECONDS)
                .build();

        String urltemp=url;
        urltemp+=lab+"/wakeup";
        Log.e(TAG,"Sending request :"+urltemp);
        //  String url = "http://10.130.150.223:5000/lab/CS101/status";
        Request request = new Request.Builder()
                .url(urltemp)
                .build();
        showSnackBarMessage("Sending WakeOnLan Request to CS101 Lab");
        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                call.cancel();
            }

            @Override
            public void onResponse(Call call, final Response response) throws IOException {
                try {
                    int httpResponseCode = response.code();
                    if (httpResponseCode == 200) {
                        final String myResponse = response.body().string();

                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                showSnackBarMessage("WakeOnLAN request to CS101 Lab complete");


                            }
                        });
                    } else {
                        showSnackBarMessage("Could not Connect with server");
                    }
                    post_processing_task();
                }
                catch(Exception e){
                    Log.e(TAG,e.toString());
                }
            }
        });
    }

    /**
     * Shut down all the PC's of Lab
     */
    private void ShutDownLab(){
        preprocessing_task();
        //ena_dis_UI_elements(false);
        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(300,TimeUnit.SECONDS)
                .writeTimeout(300,TimeUnit.SECONDS)
                .readTimeout(300,TimeUnit.SECONDS)
                .build();

        String urltemp=url;
        urltemp+=lab+"/shutdown";
        Log.e(TAG,"Sending request :"+urltemp);
        //  String url = "http://10.130.150.223:5000/lab/CS101/status";
        Request request = new Request.Builder()
                .url(urltemp)
                .build();
        showSnackBarMessage("Sending ShutDown Request to CS101 Lab");
        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                try {
                    call.cancel();
                    showSnackBarMessage("Could not connect with server");
                    //ena_dis_UI_elements(true);
                    shutdown_request = 0;
                    post_processing_task();
                }
                catch(Exception e1){
                    Log.e(TAG,e1.toString());
                }
            }

            @Override
            public void onResponse(Call call, final Response response) throws IOException {
                try{
                    int httpResponseCode=response.code();
                    if (httpResponseCode == 200) {
                        final String myResponse = response.body().string();

                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {

                                showSnackBarMessage("ShutDown Request to CS101 Lab complete");
                                closeFABMenu();
                                fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                                fab.setImageResource(R.drawable.ic_lab);

                            }
                        });
                    }
                    else{
                        showSnackBarMessage("Could not Connect with server");
                    }
                    post_processing_task();
                    shutdown_request=0;
                    //  ena_dis_UI_elements(true);
                }
                catch(Exception e){
                    Log.e(TAG,e.toString());
                }
            }
        });
    }


    /**
     * Enables the exam mode in all the PC's of lab which blocks the memory related external devices such as Pendrive from connecting to system
     */
    private void ExamModeOn(){
        preprocessing_task();
        //ena_dis_UI_elements(false);
        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(300,TimeUnit.SECONDS)
                .writeTimeout(300,TimeUnit.SECONDS)
                .readTimeout(300,TimeUnit.SECONDS)
                .build();

        String urltemp=url;
        urltemp+=lab+"/exam_mode_on";
        Log.e(TAG,"Sending request :"+urltemp);
        //  String url = "http://10.130.150.223:5000/lab/sl2/status";
        Request request = new Request.Builder()
                .url(urltemp)
                .build();
        showSnackBarMessage("Sending Exam Mode Request to SL2 Lab");
        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                try {
                    call.cancel();
                    showSnackBarMessage("Could not connect with server");
                    //ena_dis_UI_elements(true);
                    shutdown_request = 0;
                    post_processing_task();
                }
                catch(Exception e1){
                    Log.e(TAG,e1.toString());
                }
            }

            @Override
            public void onResponse(Call call, final Response response) throws IOException {
                try{
                    int httpResponseCode=response.code();
                    if (httpResponseCode == 200) {
                        final String myResponse = response.body().string();

                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {

                                showSnackBarMessage("Exam Mode setup in SL2 Lab completed");
                                closeFABMenu();
                                fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                                fab.setImageResource(R.drawable.ic_lab);

                            }
                        });
                    }
                    else{
                        showSnackBarMessage("Could not Connect with server");
                    }
                    shutdown_request=0;
                    post_processing_task();
                    //  ena_dis_UI_elements(true);
                }
                catch(Exception e){
                    Log.e(TAG,e.toString());
                }
            }
        });
    }
    /**
     * Disables the exam mode in all the PC's of lab which enables the memory related external devices such as Pendrive to be connected
     */
    private void ExamModeOff(){
        preprocessing_task();
        //ena_dis_UI_elements(false);
        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(300,TimeUnit.SECONDS)
                .writeTimeout(300,TimeUnit.SECONDS)
                .readTimeout(300,TimeUnit.SECONDS)
                .build();

        String urltemp=url;
        urltemp+=lab+"/exam_mode_off";
        Log.e(TAG,"Sending request :"+urltemp);
        //  String url = "http://10.130.150.223:5000/lab/sl2/status";
        Request request = new Request.Builder()
                .url(urltemp)
                .build();
        showSnackBarMessage("Disabling Exam Mode in CS101 Lab");
        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                try {
                    call.cancel();
                    showSnackBarMessage("Could not connect with server");
                    //ena_dis_UI_elements(true);
                    shutdown_request = 0;
                    post_processing_task();
                }
                catch(Exception e1){
                    Log.e(TAG,e1.toString());
                }
            }

            @Override
            public void onResponse(Call call, final Response response) throws IOException {
                try{
                    int httpResponseCode=response.code();
                    if (httpResponseCode == 200) {
                        final String myResponse = response.body().string();

                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {

                                showSnackBarMessage("Exam Mode disabled in CS101 Lab");
                                closeFABMenu();
                                fab.setBackgroundTintList(ColorStateList.valueOf(getResources().getColor(R.color.colorPrimaryDark)));
                                fab.setImageResource(R.drawable.ic_lab);

                            }
                        });
                    }
                    else{
                        showSnackBarMessage("Could not Connect with server");
                    }
                    shutdown_request=0;
                    post_processing_task();
                    //  ena_dis_UI_elements(true);
                }
                catch(Exception e){
                    Log.e(TAG,e.toString());
                }
            }
        });
    }


    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        // Do something that differs the Activity's menu here
        super.onCreateOptionsMenu(menu, inflater);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {

            case R.id.exam_mode_on:
                ExamModeOn();

                break;
            case R.id.exam_mode_off:
                ExamModeOff();
                break;

            default:
                break;
        }

        return false;
    }
}