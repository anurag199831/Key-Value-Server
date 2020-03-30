package com.example.labstatus.ui.SL2;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;
/** @brief Model for SL1 grid data

Detailed description follows here.
 @author Area51
 @date Nov 2019
 */
public class SL2ViewModel extends ViewModel {

    private MutableLiveData<String> mText;



    public String text;

    public String color;
    /**
     * Constructor for Model
     * @param c color
     * @param t text
     */
    public SL2ViewModel(String t, String c) {
       // mText = new MutableLiveData<>();
        //mText.setValue("This is SL2 fragment");
    text=t;
    color=c;
    }

    public SL2ViewModel() {
         mText = new MutableLiveData<>();
        mText.setValue("This is SL2 fragment");

    }

    public LiveData<String> getText() {
        return mText;
    }
}